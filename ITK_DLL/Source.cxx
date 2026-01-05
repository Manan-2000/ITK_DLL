#include"Header.hxx"
#define err1 919002
#define err2 919003


fstream fobj;

int ITK_DLL_register_callbacks() {
	int status = ITK_ok;
	fobj.open("C:\\Users\\FaithPLM\\Desktop\\Test files\\DLL_Print.txt", ios::out);
	fobj << "DLL CALL REGISTERED"<<endl;
	status = CUSTOM_register_exit("ITK_DLL","USER_init_module",(CUSTOM_EXIT_ftn_t)CUSTOM_EXIT);

	return status;
}

int CUSTOM_EXIT (int* n, va_list list) {
	int status = ITK_ok;
	fobj << "\n\nDLL EXIT CALL REGISTERED, LOGIN SUCCESS";
	//status = EPM_register_rule_handler("Check-target-count", "Check Target Count",(EPM_rule_handler_t)check_target_count);
	//status = EPM_register_rule_handler("Check-release-&-PDF-dataset", "Check release & PDF dataset", (EPM_rule_handler_t)check_rel_and_PDF);
	//status = EPM_register_rule_handler("IR-and-all-child-should-have-PDF", "IR-and-all-child-should-have-PDF", (EPM_rule_handler_t)check_IR_and_child_PDF);
	//status = EPM_register_rule_handler("Validate-Dataset-Named-Reference", "Check to see if the Dataset contain a Named Reference", (EPM_rule_handler_t)check_named_ref);
	//status = EPM_register_action_handler("Assign-Reviewer", "Assign Reviewer based on Item ID", (EPM_action_handler_t)set_reviewer);
	//status = EPM_register_action_handler("Delete-PDF", "Delete PDF assigned to attachment", (EPM_action_handler_t)del_pdf);
	
	
	METHOD_id_t method_id;
	//status = METHOD_find_method("ItemRevision", TC_delete_msg,&method_id);
	//if (method_id.id == 0) {
	//	fobj << "Method is not registered for specified Message/Type" << endl;
	//	fobj.close();
	//	return 0;
	//}
	//status = METHOD_add_action(method_id, METHOD_pre_action_type,(METHOD_function_t)dataset_backup,NULL);

	status = METHOD_find_method("PDF", AE_create_dataset_msg, &method_id);
	if (method_id.id == 0) {
		fobj << "Method is not registered for specified Message/Type" << endl;
		fobj.close();
		return 0;
	}
	status = METHOD_add_action(method_id, METHOD_post_action_type, (METHOD_function_t)dataset_release, NULL);

	return status;
}


EPM_decision_t check_target_count(EPM_rule_message_t msg) {

	EPM_decision_t decision = EPM_go;
	int status = ITK_ok;

	tag_t root_task = NULLTAG;
	status = EPM_ask_root_task(msg.task, &root_task);
	
	int count = 0;
	scoped_smptr<tag_t> attachments;
	status = EPM_ask_attachments(root_task, EPM_target_attachment, &count, &attachments);

	int target_count = 0;
	for (int i = 0; i < count; i++) {
		scoped_smptr<char> type;
		status = AOM_ask_value_string(attachments[i], "object_type", &type);

		if (tc_strcmp(type.get(), "ItemRevision") == 0) {
			target_count++;
		}
		if (target_count > 1) {
			decision = EPM_nogo;
			break;
		}
	}
	fobj << "\n\nTARGET COUNT = "<<target_count;
	fobj.close();
	return decision;
}

EPM_decision_t check_rel_and_PDF(EPM_rule_message_t msg) {
	
	EPM_decision_t decision = EPM_nogo;
	int status = ITK_ok;

	tag_t root_task = NULLTAG;
	status = EPM_ask_root_task(msg.task, &root_task);

	int count = 0;
	scoped_smptr<tag_t> attachments;
	status = EPM_ask_attachments(root_task, EPM_target_attachment, &count, &attachments);

	tag_t PDF_type = NULLTAG;
	status = TCTYPE_find_type("PDF", "Dataset", &PDF_type);

	for (int i = 0; i < count; i++) {
		
		int is_released = 0;
		status = EPM_ask_if_released(attachments[i], &is_released);
		if (is_released == 1) {
			return decision;
		}

		int exit = 0;

		int secondary_count = 0;
		scoped_smptr<tag_t> secondary_obj;
		tag_t obj_type = NULLTAG;
		status = GRM_list_secondary_objects_only(attachments[i], NULLTAG , &secondary_count, &secondary_obj);
		if (secondary_count < 2) {
			return decision;

		}
		for (int j = 0; j < secondary_count; j++) {
			status = TCTYPE_ask_object_type(secondary_obj[j], &obj_type);

			if (obj_type == PDF_type) {
				exit++;
				break;
			}
		}
		if (exit > 0) {
			decision = EPM_go;
			break;
		}
		
	}

	return decision;
}

EPM_decision_t check_IR_and_child_PDF(EPM_rule_message_t msg) {

	EPM_decision_t decision = EPM_go;
	int status = ITK_ok;

	tag_t root_task = NULLTAG;
	status = EPM_ask_root_task(msg.task, &root_task);

	int count = 0;
	scoped_smptr<tag_t> attachments;
	status = EPM_ask_attachments(root_task, EPM_target_attachment, &count, &attachments);

	for (int i = 0; i < count; i++) {
		status = BOM(attachments[i]);
		if (status != ITK_ok) {
			decision = EPM_nogo;
			break;
		}
	}
	if (decision == EPM_nogo) {
		scoped_smptr<char>err_text;
		EMH_ask_error_text(err1, &err_text);
		status = EMH_store_error_s1(EMH_severity_error, err1, err_text.get());
	}
	return decision;
}

EPM_decision_t check_named_ref(EPM_rule_message_t msg) {

	EPM_decision_t decision = EPM_go;
	int status = ITK_ok;

	tag_t root_task = NULLTAG;
	status = EPM_ask_root_task(msg.task, &root_task);

	int count = 0;
	scoped_smptr<tag_t> attachments;
	status = EPM_ask_attachments(root_task, EPM_target_attachment, &count, &attachments);
	int dataset_count = 0;
	int total_name_ref = 0;
	for (int i = 0; i < count; i++) {
		int sec_count = 0;
		scoped_smptr<tag_t>secondary_objects;
		status = GRM_list_secondary_objects_only(attachments[i], NULLTAG, &sec_count,&secondary_objects);
		dataset_count += sec_count-1;
		int name_ref_count = 0;
		for (int j = 0; j < sec_count; j++) {
			int nFound = 0;
			scoped_smptr<tag_t>refObject;
			AE_ask_dataset_named_refs(secondary_objects[j], &nFound, &refObject);
			if (nFound != 0) {
				name_ref_count++;
			}
		}
		total_name_ref += name_ref_count;

	}
	if (dataset_count == 0) {
		decision = EPM_nogo;
	}

	if (dataset_count != total_name_ref) {
		decision = EPM_nogo;
		scoped_smptr<char>err_text;
		EMH_ask_error_text(err2, &err_text);
		status = EMH_store_error_s1(EMH_severity_error, err2, err_text.get());
	}

	return decision;
}

int set_reviewer(EPM_action_message_t msg) {
	int status = ITK_ok;

	tag_t root_task = NULLTAG;
	status = EPM_ask_root_task(msg.task, &root_task);

	int count = 0;
	scoped_smptr<tag_t>attachments;
	status = EPM_ask_attachments(root_task, EPM_target_attachment, &count, &attachments);

	for (int i = 0; i < count; i++) {
		
		scoped_smptr<char>value;
		status = AOM_ask_value_string(attachments[i], "item_id", &value);

		if (strstr(value.get(), "IN")) {
			status = EPM_set_task_result(msg.task, "IN");
		}

		else if (strstr(value.get(), "US")) {
			status = EPM_set_task_result(msg.task, "US");
		}
		else {
			status = EPM_set_task_result(msg.task, "Global");
		}
	}

	return status;
}

int del_pdf(EPM_action_message_t msg) {
	int status = ITK_ok;
	
	tag_t root_task = NULLTAG;
	status = EPM_ask_root_task(msg.task, &root_task);

	int count = 0;
	scoped_smptr<tag_t>attachments;
	status = EPM_ask_attachments(root_task, EPM_target_attachment, &count, &attachments);

	int rel_count = 0;
	scoped_smptr<tag_t>relation_type_list;
	GRM_list_relation_types(&rel_count, &relation_type_list);
	int exit = 0;
	
	for (int i = 0; i < count; i++) {

		int sec_count = 0;
		scoped_smptr<tag_t> secondary_list;
		status = GRM_list_secondary_objects_only(attachments[i], NULLTAG, &sec_count, &secondary_list);

		if (sec_count > 1) {
			for (int j = 0; j < sec_count; j++) {
				scoped_smptr<char>obj_type;
				status = WSOM_ask_object_type2(secondary_list[j], &obj_type);

				if (tc_strcmp(obj_type.get(), "PDF") == 0) {
					for (int k = 0; k < rel_count; k++) {
						tag_t relation = NULLTAG;
						status = GRM_find_relation(attachments[i], secondary_list[j], relation_type_list[k], &relation);
						ERROR_Handling;
						if (relation != NULLTAG) {
							status = GRM_delete_relation(relation);
							ERROR_Handling;
							exit++;
							break;
						}
					}
				}
				if (exit > 0) {
					break;
				}
			}
		}
		if (exit > 0) {
			break;
		}

	}


	return status;
}
	
int dataset_backup(METHOD_message_t* msg, va_list list) {
	int status = ITK_ok;
	tag_t rev = va_arg(list, tag_t);
	int count = 0;
	scoped_smptr<tag_t>sec_obj;
	status = GRM_list_secondary_objects_only(rev, NULLTAG, &count, &sec_obj);
	for (int i = 0; i < count; i++) {
		scoped_smptr<char>obj_type;
		status = WSOM_ask_object_type2(sec_obj[i], &obj_type);
		if (tc_strcmp(obj_type.get(), "PDF") == 0) {
			scoped_smptr<char>value;
			status = AOM_ask_value_string(sec_obj[i], "object_name", &value);
			string name = string(value.get()) + ".pdf";

			tag_t new_dataset = NULLTAG;
			status = AE_copy_dataset_with_id(sec_obj[i], name.c_str(), name.c_str(), name.c_str(), &new_dataset);
			tag_t curr_grp_mem = NULLTAG;
			status = SA_ask_current_groupmember(&curr_grp_mem);
			tag_t user_tag = NULLTAG;
			status = SA_ask_groupmember_user(curr_grp_mem, &user_tag);
			tag_t newfolder = NULLTAG;
			status = SA_ask_user_newstuff_folder(user_tag, &newfolder);

			status = FL_insert(newfolder, new_dataset, 999);

			status = AOM_save_without_extensions(newfolder);

		}
	}
	return status;
}

int dataset_release(METHOD_message_t* msg, va_list list) {
	int status = ITK_ok;

	tag_t  datasetTypeTag = va_arg(list, tag_t);
	const char* datasetName = va_arg(list, char*);
	const char* datasetDescription = va_arg(list, char*);
	const char* datasetID = va_arg(list, char*);
	const char* datasetRevision = va_arg(list, char*);
	tag_t newDataset = va_arg(list, tag_t);
	
	scoped_smptr<char>obj_type;
	status = AOM_ask_value_string(newDataset,"object_type", &obj_type);
	if (tc_strcmp(obj_type.get(), "PDF") == 0) {
		int primary_count = 0;
		scoped_smptr<tag_t>primary_objects;
		status = GRM_list_primary_objects_only(newDataset, NULLTAG, &primary_count, &primary_objects);
		ERROR_Handling;
		int count = 0;
		tag_t relation_type = NULLTAG;
		status = GRM_find_relation_type("IMAN_manifestation", &relation_type);
		tag_t relation = NULLTAG;
		status = GRM_find_relation(primary_objects[0], newDataset, relation_type, &relation);
		ERROR_Handling;
		if (relation != NULLTAG) {
			scoped_smptr<char>value;
			status = AOM_ask_value_string(primary_objects[0], "release_status_list", &value);
			tag_t release_status;
			status = EPM_find_status_type(value.get(), &release_status);
			
			tag_t* dataset2 = (tag_t*)MEM_alloc(sizeof(tag_t));
            dataset2[0] = newDataset;
            status = RELSTAT_add_release_status(release_status, 1, dataset2, TRUE);
            MEM_free(dataset2);
        }
    }

    return status;
}

//int dataset_release(METHOD_message_t* msg, va_list list) {
//	int status = ITK_ok;
//	tag_t  primary_object = va_arg(list, tag_t);
//	tag_t  secondary_object = va_arg(list, tag_t);
//	tag_t  relation_type = va_arg(list, tag_t);
//	
//	scoped_smptr<char> primary_type;
//	scoped_smptr<char> secondary_type;
//	status = AOM_ask_value_string(primary_object, "object_type", &primary_type);
//	status = AOM_ask_value_string(secondary_object, "object_type", &secondary_type);
//	tag_t manifestation = NULLTAG;
//	status = GRM_find_relation_type("IMAN_manifestation", &manifestation);
//
//	if (tc_strcmp(secondary_type.get(), "PDF") == 0 and (relation_type == manifestation)) {
//		scoped_smptr<char> value;
//		status = AOM_ask_value_string(primary_object, "release_status_list", &value);
//
//		tag_t status = NULLTAG;
//		status = EPM_find_status_type(value.get(), &status);
//
//		tag_t* secondary = (tag_t*)MEM_alloc(sizeof(tag_t*));
//		secondary[0] = secondary_object;
//		status = RELSTAT_add_release_status(status, 1, secondary, TRUE);
//
//		MEM_free(secondary);
//	}
//
//	return status;
//}
