/* min_scripter_mod.c -- Min scripter plugin interface for watchdog tests

   Copyright (C) 2000-2010, Nokia Corporation.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, version 2.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <test_module_api.h>
#include <min_parser.h>
#include <min_logger.h>
#include <min_test_event_if.h>
#include <min_ipc_mechanism.h>

#include <limits.h>

#include "blts_watchdog_util.h"


/* Min calls getters for these variables (see below): */
char *module_date = __DATE__;
char *module_time = __TIME__;
TTestModuleType module_type     = ETestClass;
unsigned int    module_version  = 200924;


/* Required definitions, for Min scripter interface. From Min template. */
/* snip >>>-------------------------------------------------------------*/

#ifdef ENTRYTC
#undef ENTRYTC
#endif
#define ENTRYTC(_l_, _n_,_f_)                                           \
        do {                                                            \
        TestCaseInfoTC* tc = (TestCaseInfoTC*)malloc(sizeof(TestCaseInfoTC));\
        if( tc == NULL ) break;                                         \
        STRCPY(tc->name_,_n_,MaxTestCaseName);                          \
        tc->test_ = _f_;                                                \
        tc->id_   = dl_list_size(_l_)+1;                                \
        dl_list_add( _l_, (void*)tc );                                  \
        } while(0)

typedef struct _TestCaseInfoTC TestCaseInfoTC;
typedef struct _ScriptVariable ScriptVariable;

typedef int (*ptr2testtc)( MinItemParser * tcr );

struct _TestCaseInfoTC
{
        char name_[MaxTestCaseName];
        ptr2testtc test_;
        unsigned int id_;
};

struct _ScriptVariable
{
        char *var_name_;
        TSBool is_initialized_;
        char *var_value_;
};

/* <<< snip ------------------------------------------------------------*/

/* Forward declarations */

int test_watchdog_presence(MinItemParser *args);
int test_open_close(MinItemParser *args);
int test_send_keepalive(MinItemParser *args);

/* Required functions for a Min scripter plugin. --->>> */

/* Min calls this when it needs a list of test cases implemented by us. */
int ts_get_test_cases(DLList ** list)
{
	ENTRYTC(*list, "test_watchdog_presence", test_watchdog_presence);
	ENTRYTC(*list, "test_open_close", test_open_close);
	ENTRYTC(*list, "test_send_keepalive", test_send_keepalive);
	return 0;
}

/* Getters for module id data */
unsigned int get_module_type() { return module_type; }
unsigned int get_module_version() { return module_version; }
char* get_module_date() { return module_date; }
char* get_module_time() { return module_time; }

/* Comparison helper from Min template. */
int _look4callname (const void *a, const void *b)
{
        TestCaseInfoTC *tci = (TestCaseInfoTC *) a;
        return strcmp (tci->name_, (char *)b);
}

/* Min TC runner (from template, shm var passing removed) */
int ts_run_method (MinItemParser * item)
{
        DLList         *l;
        DLListIterator  it;
        int             retval;
        char           *callname;

        l = dl_list_create ();
        callname = INITPTR;

        retval = mip_get_next_string (item, &callname);
        if (retval != ENOERR) {
                retval = -1;
                goto EXIT;
        }

        ts_get_test_cases (&l);
        it = dl_list_find (dl_list_head (l)
                           , dl_list_tail (l)
                           , _look4callname, callname);

        if (it == DLListNULLIterator) {
                retval = -1;
                DELETE (callname);
                it = dl_list_head (l);
                while (it != DLListNULLIterator) {
                        free (dl_list_data (it));
                        dl_list_remove_it (it);
                        it = dl_list_head (l);
                }
                dl_list_free (&l);
                goto EXIT;
        }

        retval = ((TestCaseInfoTC *) dl_list_data (it))->test_ (item);

        DELETE (callname);
        it = dl_list_head (l);
        while (it != DLListNULLIterator) {
                free (dl_list_data (it));
                dl_list_remove_it (it);
                it = dl_list_head (l);
        }
        dl_list_free (&l);
EXIT:
        return retval;
}

/* These are scripter-callable functions for individual components. >>> */

int test_watchdog_presence(MinItemParser *args)
{
	char* logfile = NULL;
	int execution_time = 30;
	int test_result;

	if(mip_set_parsing_type(args, EQuoteStyleParsing) < 0) goto parse_error;

	if(mip_get_next_string(args, &logfile) < 0) goto parse_error;
	if(mip_get_next_int(args, &execution_time) < 0) goto parse_error;

	log_open(logfile, 0);

	test_result = wdt_dep_check();
	if(test_result)
	{
		log_print(" *** Test execution failed ***\n");
	}
	log_close();
	free(logfile);

	return test_result;

parse_error:
	tm_printf(1, "", "Parse error in %s",__FUNCTION__);
	if(logfile) free(logfile);
	return EINVAL;
}

int test_open_close(MinItemParser *args)
{
	char* logfile = NULL;
	int execution_time = 30;
	int test_result;

	if(mip_set_parsing_type(args, EQuoteStyleParsing) < 0) goto parse_error;

	if(mip_get_next_string(args, &logfile) < 0) goto parse_error;
	if(mip_get_next_int(args, &execution_time) < 0) goto parse_error;

	log_open(logfile, 0);

	test_result = wdt_open_close();
	if(test_result)
	{
		log_print(" *** Test execution failed ***\n");
	}
	log_close();
	free(logfile);

	return test_result;

parse_error:
	tm_printf(1, "", "Parse error in %s",__FUNCTION__);
	if(logfile) free(logfile);
	return EINVAL;
}

int test_send_keepalive(MinItemParser *args)
{
	char* logfile = NULL;
	int execution_time = 30;
	int test_result;

	if(mip_set_parsing_type(args, EQuoteStyleParsing) < 0) goto parse_error;

	if(mip_get_next_string(args, &logfile) < 0) goto parse_error;
	if(mip_get_next_int(args, &execution_time) < 0) goto parse_error;

	log_open(logfile, 0);

	test_result = wdt_send_keepalive(execution_time);
	if(test_result)
	{
		log_print(" *** Test execution failed ***\n");
	}
	log_close();
	free(logfile);

	return test_result;

parse_error:
	tm_printf(1, "", "Parse error in %s",__FUNCTION__);
	if(logfile) free(logfile);
	return EINVAL;
}

