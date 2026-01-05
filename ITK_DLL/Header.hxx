#include<tc/tc_startup.h>
#include<tccore/custom.h>
#include<epm/epm.h>
#include<tccore/grm.h>
#include<bom/bom.h>
#include<bom/bomlines.h>
#include<bom/bom_attr.h>
#include<tc/emh.h>
#include<ae/dataset.h>
#include<fstream>
#include<base_utils/ScopedSmPtr.hxx>
#include<tccore/aom.h>
#include<tccore/aom_prop.h>
#include<fclasses/tc_string.h>
#include<tccore/tc_msg.h>
#include<sa/group.h>
#include<tc/folder.h>
#include<ae/dataset_msg.h>
#include<tccore/grm_msg.h>
using namespace std;
using namespace Teamcenter;

#define ERROR_Handling if(status != ITK_ok) {scoped_smptr<char> text;  EMH_ask_error_text(status,&text);fstream fobj; fobj.open("C:\\Users\\FaithPLM\\Desktop\\Test files\\Errors.txt",ios::app); fobj<<text.get()<<endl; fobj.close();}


extern "C"{
	_declspec(dllexport) int ITK_DLL_register_callbacks();
}

int CUSTOM_EXIT(int* n, va_list list);
EPM_decision_t check_target_count(EPM_rule_message_t msg);
EPM_decision_t check_rel_and_PDF(EPM_rule_message_t msg);
EPM_decision_t check_IR_and_child_PDF(EPM_rule_message_t msg);
EPM_decision_t check_named_ref(EPM_rule_message_t msg);
int set_reviewer(EPM_action_message_t msg);
int del_pdf(EPM_action_message_t msg);
int dataset_backup(METHOD_message_t* msg, va_list list);
int dataset_release(METHOD_message_t* msg, va_list list);



int Check_PDF(tag_t rev);
int BOM_get_child(tag_t topline);
int BOM(tag_t rev);



