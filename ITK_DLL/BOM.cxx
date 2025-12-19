#include"Header.hxx"

int BOM_get_child(tag_t topline) {
	int status = ITK_ok;
	//int pdf = 1;
	int count = 0;
	scoped_smptr<tag_t> child;
	status = BOM_line_ask_child_lines(topline, &count, &child);
	status = Check_PDF(topline);
	if (status != 0) {
		return status;
	}
	for (int i = 0; i < count; i++) {

		BOM_get_child(child[i]);
		
	}
	return status;
}

int BOM(tag_t rev) {
	int status = ITK_ok;
	tag_t window = NULLTAG, topline = NULLTAG;
	status = BOM_create_window(&window);
	status = BOM_set_window_top_line(window, NULLTAG, rev, NULLTAG, &topline);
	status = BOM_get_child(topline);
	return status;
}