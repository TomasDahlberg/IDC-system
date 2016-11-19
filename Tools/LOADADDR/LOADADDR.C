#include <stdio.h>
#include <dir.h>
#include <fcntl.h>
#include <io.h>

long filesize(char *file)
{
  int handle;
  if ((handle = open(file, O_RDONLY)) == -1)
    return 0;
  return filelength(handle);
}

int main(int argc, char *argv[])
{
  FILE *fp;
  char buf[81], prefix1[80], prefix2[80], suffix[80];
  long load_address, fs;

  if ((fp = fopen("ladda.bat", "r")) == NULL)
    return 0;	/* no such file or some other error */
  if (fgets(buf, 80, fp) == NULL) {
    fclose(fp);
    return 0;
  }
  fclose(fp);

/*  "sendmod big.exe 1212"  */

  if (sscanf(buf, "%s %s %lx %s", prefix1, prefix2, &load_address, suffix)
		!= 4)
    suffix[0] = 0;
/*
  sscanf(&buf[16], "%lx", &load_address);
*/
  fs = filesize("big.exe");
  if (fs + load_address > 0x3c000) {  /* load high */
    load_address = 0x60000;
    printf("Varning: Extraminne kr„vs, laddadress = $60000\n");
  }
  if ((fp = fopen("ladda.bat", "w")) == NULL)
    return 0;	/* some strange error */
  fprintf(fp, "%s %s %lx %s %%1 %%2\n", prefix1, prefix2, load_address,
					argc == 2 ? argv[1] : suffix);
  fclose(fp);
  return (load_address == 0x60000);	/* 1 == modified, otherwise == 0 */
}