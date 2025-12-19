#include"Header.hxx"

int Check_PDF(tag_t topline) {
	int status = ITK_ok;
	tag_t pdf_type = NULLTAG;
	status = TCTYPE_find_type("PDF", "Dataset", &pdf_type);
	scoped_smptr<char>value;
	status = AOM_ask_value_string(topline, bomAttr_itemId, &value);
	tag_t qry = NULLTAG;
	status = QRY_find2("Item ID", &qry);


	char** entries = (char**)MEM_alloc(sizeof(char**));
	entries[0] = (char*)MEM_alloc(sizeof(char*) * tc_strlen("Item ID"));
	tc_strcpy(entries[0], "Item ID");

	char** values = (char**)MEM_alloc(sizeof(char**));
	values[0] = (char*)MEM_alloc(sizeof(char*) * tc_strlen(value.get()));
	tc_strcpy(values[0], value.get());

	int nfind = 0;
	tag_t* results = NULL;

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

	if (results) {
		MEM_free(results);
	}

	scoped_smptr<tag_t>secondary_objects, types;
	int pdf_found = 1;

	int secondary_count = 0;
	scoped_smptr<tag_t> secondary_obj;
	tag_t obj_type = NULLTAG;
	status = GRM_list_secondary_objects_only(rev, NULLTAG, &secondary_count, &secondary_obj);
	if (secondary_count < 2) {
			return status++;

	}
	for (int j = 0; j < secondary_count; j++) {
		status = TCTYPE_ask_object_type(secondary_obj[j], &obj_type);

		if (obj_type == pdf_type) {
			pdf_found = 0;
			break;
		}
	}
	if (pdf_found != 0) {
		status++;
	}
	return status;
}
