/* ***** BEGIN LICENSE BLOCK *****
 *
 * The contents of this file is copyrighted by Thomas and Hans-Joerg Frieden.
 * It's content is not open source and may not be redistributed, modified or adapted
 * without permission of the above-mentioned copyright holders.
 *
 * Since this code was originally developed under an AmigaOS related bounty, any derived
 * version of this file may only be used on an official AmigaOS system.
 *
 * Contributor(s):
 * 	Thomas Frieden <thomas@friedenhq.org>
 * 	Hans-Joerg Frieden <hans-joerg@friedenhq.org>
 *
 * ***** END LICENSE BLOCK ***** */

 #include "primpl.h"

#include <stdlib.h>
#include <stdio.h>
#include <proto/dos.h>

PR_IMPLEMENT(char *) _amigaos_GetEnv(const char *name)
{
	char *res = getenv(name);
//	fprintf(stderr, "_amigaos_GetEnv: %s = %s\n", name, res);
	return res;
}

PR_IMPLEMENT(PRIntn) _amigaos_PutEnv(const char *name)
{
//	fprintf(stderr, "_amigaos_PutEnv: %s\n", name);
    PRIntn result = putenv(name);

    /* Need to write through to AmigaOS Env: to get the variables to transport
     * over into subprocesses.
     */

    char *var = strdup(name);
    if (!var)
    	return result;

    char *val = strchr(var, '=');

    if (val)
    {
    	*val = 0;
    	val++;
    }
    else
    	val = "";

    IDOS->SetVar(var, val, strlen(val), GVF_LOCAL_ONLY);

    free(var);
    return result;
}

PR_IMPLEMENT(PRSize) _amigaos_GetRandomNoise(void *buf, PRSize  size)
{
    FILE *f = fopen("RANDOM:", "rb");
    if (f)
    {
    	PRSize s = (PRSize)fread(buf, 1, (size_t)size, f);
    	fclose(f);
        return s;
    }
    else
    {
    	PRSize i;
    	char *buffer = (char *)buf;
    	for (i = 0; i < size; i++)
    		buffer[i] = rand() & 0xff;

    	return size;
    }

    return 0;
}

