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
	status = EPM_register_rule_handler("Validate-Dataset-Named-Reference", "Check to see if the Dataset contain a Named Reference", (EPM_rule_handler_t)check_named_ref);

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
