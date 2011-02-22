#include "unrar.h"

#include <stdlib.h>
#include <string.h>

static int 
_unrar_test_password_callback(unsigned int msg, long data, long P1, long P2)
{
        switch(msg) {
                case UCM_NEEDPASSWORD:
                        *(int *)data = 1;
                        break;
                default:
                        return 0;
        }
        return 0;
}


int
unrar_test_password(const char * file, const char * pwd) 
{
        void                      * unrar;
        struct RARHeaderData        headerdata;
        struct RAROpenArchiveData   archivedata;
        int                         password_incorrect;

        password_incorrect = 0;
        memset(&headerdata, 0, sizeof(headerdata));
        memset(&archivedata, 0, sizeof(archivedata));
        archivedata.ArcName  = (char *) file;
        archivedata.OpenMode = RAR_OM_EXTRACT;

        unrar = RAROpenArchive(&archivedata);
        if (!unrar || archivedata.OpenResult)
                return -2;

        RARSetPassword(unrar, (char *) pwd);

        RARSetCallback(unrar, _unrar_test_password_callback, (long) &password_incorrect);

        if (RARReadHeader(unrar, &headerdata) != 0)
                goto error;

        if (RARProcessFile(unrar, RAR_TEST, NULL, NULL) != 0)
                goto error;

        if (password_incorrect)
                goto error;

        RARCloseArchive(unrar);
        return 0;

error:
        RARCloseArchive(unrar);
        return -1;
}

