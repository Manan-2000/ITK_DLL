#include"Header.hxx"
#define err1 919002
#define err2 919003
#define err3 919004


fstream fobj;

EPM_decision_t Check_PDF(tag_t topline) {
	int status = ITK_ok;
	EPM_decision_t decision = EPM_nogo;
	tag_t pdf_type = NULLTAG;
	status = TCTYPE_find_type("PDF", "Dataset", &pdf_type);
	scoped_smptr<char>value;
	status = AOM_ask_value_string(topline, "Item ID", &value);
	tag_t qry = NULLTAG;
	status = QRY_find2("Item ID", &qry);


	char** entries = (char**)MEM_alloc(sizeof(char**));
	entries[0] = (char*)MEM_alloc(sizeof(char*) * tc_strlen("Item ID"));
	tc_strcpy(entries[0], "Item ID");

	char** values = (char**)MEM_alloc(sizeof(char**));
	values[0] = (char*)MEM_alloc(sizeof(char*) * tc_strlen(value.get()));
	tc_strcpy(values[0], value.get());

	int nfind = 0;
	scoped_smptr<tag_t>results;

	status = QRY_execute(qry, 1, entries, values, &nfind, &results);

	if (entries[0]) {
		MEM_free(entries[0]);
	}
	if (entries) {
		MEM_free(entries);
	}
	if (values[0]) {
		MEM_free(values[0]);
	}
	if (values) {
		MEM_free(values);
	}

	tag_t rev = NULLTAG;
	status = ITEM_ask_latest_rev(results[0], &rev);
	
	int count = 0;
	scoped_smptr<tag_t>secondary_objects,types;
	status = GRM_list_secondary_objects_only(rev, NULLTAG, &count, &secondary_objects);
	status = TCTYPE_ask_object_types(count, secondary_objects.get(),&types);
	for (int i = 0; i < count; i++) {
		if (types[i] == pdf_type) {
			decision = EPM_go;
			break;
		}
	}
	return decision;
}

EPM_decision_t BOM(tag_t rev) {
	int status = ITK_ok;
	EPM_decision_t decision = EPM_nogo;
	tag_t window = NULLTAG, topline = NULLTAG;
	status = BOM_create_window(&window);
	status = BOM_set_window_top_line(window, NULLTAG, rev, NULLTAG, &topline);

	int count = 0;
	scoped_smptr<tag_t> child;
	status = BOM_line_ask_child_lines(topline, &count, &child);
	decision = Check_PDF(topline);
	for (int i = 0; i < count; i++) {
		if (decision == EPM_go) {
			break;
		}
		BOM(child[i]);
		
	}
	return decision;
}

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
	status = EPM_register_rule_handler("IR-and-one-child-should-have-PDF", "IR-and-one-child-should-have-PDF", (EPM_rule_handler_t)check_IR_and_child_PDF);

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

	EPM_decision_t decision = EPM_nogo;
	int status = ITK_ok;

	tag_t root_task = NULLTAG;
	status = EPM_ask_root_task(msg.task, &root_task);

	int count = 0;
	scoped_smptr<tag_t> attachments;
	status = EPM_ask_attachments(root_task, EPM_target_attachment, &count, &attachments);

	for (int i = 0; i < count; i++) {
		decision = BOM(attachments[i]);
	}
	scoped_smptr<char>err_text;
	EMH_ask_error_text(err1,&err_text);
	status = EMH_store_error_s1(EMH_severity_error, err1, err_text.get());

	return decision;
}
