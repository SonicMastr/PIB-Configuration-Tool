#include <curl/curl.h>
#include <archive.h>
#include <archive_entry.h>
#include "common.h"
#include "gxm.h"
#include "ctrl.h"

extern unsigned char _binary_resources_libshacccg_suprx_start;
extern unsigned char _binary_resources_libshacccg_suprx_size;
extern unsigned char _binary_resources_libScePiglet_suprx_start;
extern unsigned char _binary_resources_libScePiglet_suprx_size;

int CallImeDialog(SceImeDialogParam *param){
	int ret;
	
	param->sdkVersion = 0x03150021,
	ret = sceImeDialogInit(param);

	if(ret < 0){
		printf("show dialog failed!: %x\n", ret);
		return ret;
	}

	while(1){
		ret = sceImeDialogGetStatus();
		if(ret < 0){
			break;

		}else if(ret == SCE_COMMON_DIALOG_STATUS_FINISHED){

			SceImeDialogResult result;
			memset(&result, 0, sizeof(result));

			sceImeDialogGetResult(&result);

			if(result.button == SCE_IME_DIALOG_BUTTON_CLOSE){
                result.result = "9"; // Dumb Fix for a bug
			}
			break;
		}

		gxm_swap();
        sceDisplayWaitVblankStart();
	}

	sceImeDialogTerm();

	return ret;
}

static void utf16_to_utf8(const uint16_t *src, uint8_t *dst) {
  int i;
  for (i = 0; src[i]; i++) {
    if ((src[i] & 0xFF80) == 0) {
      *(dst++) = src[i] & 0xFF;
    } else if((src[i] & 0xF800) == 0) {
      *(dst++) = ((src[i] >> 6) & 0xFF) | 0xC0;
      *(dst++) = (src[i] & 0x3F) | 0x80;
    } else if((src[i] & 0xFC00) == 0xD800 && (src[i + 1] & 0xFC00) == 0xDC00) {
      *(dst++) = (((src[i] + 64) >> 8) & 0x3) | 0xF0;
      *(dst++) = (((src[i] >> 2) + 16) & 0x3F) | 0x80;
      *(dst++) = ((src[i] >> 4) & 0x30) | 0x80 | ((src[i + 1] << 2) & 0xF);
      *(dst++) = (src[i + 1] & 0x3F) | 0x80;
      i += 1;
    } else {
      *(dst++) = ((src[i] >> 12) & 0xF) | 0xE0;
      *(dst++) = ((src[i] >> 6) & 0x3F) | 0x80;
      *(dst++) = (src[i] & 0x3F) | 0x80;
    }
  }

  *dst = '\0';
}

bool fileExists(char *fileName, char *dir)
{
    struct SceIoStat *fileStat = (SceIoStat*)malloc(sizeof(SceIoStat));

    int dirLen = strlen(dir);
    int fileLen = strlen(fileName);
    char filePath[dirLen + fileLen + 2];
    memset(filePath, '\0', sizeof(char)*(dirLen + fileLen + 2));

    strcpy(filePath, dir);
    if(strcmp(&filePath[dirLen - 1], "/"))
        strcat(filePath, "/");
    strcat(filePath, fileName);

    if(sceIoGetstat(filePath, fileStat) < 0) {
        free(fileStat);
        return false;
    } else {
        free(fileStat);
        return true;
    }
}

int saveFile(char *data, char *size, char *filepath, char *name)
{
    LOG("Installing %s\n", name);
    FILE *fp;
    fp = fopen(filepath, "wb");
    if (fp)
    {
        fwrite(data, size, 1, fp);
        fclose(fp);
        SUCCESS("Installed %s\n", name);
        return 0;
    } else {
        ERROR("Failed to Install %s\n", name);
        return -1;
    }
}

void createDirectory(char *dir)
{
    struct SceIoStat *dirStat = (SceIoStat*)malloc(sizeof(SceIoStat));
    if(sceIoGetstat(dir, dirStat) < 0){
		sceIoMkdir(dir , 0777);
        LOG("Created Directory %s\n", dir);
    } else {
        LOG("Directory Exists. Skipping...\n");
    }
    free(dirStat);
}

bool hasResolutionConfig()
{
    if (fileExists("resolution.bin", "ur0:data/external"))
        return true;
    else
        return false;
}

char *getResolutionConfig()
{
    int mode;
    FILE *fp;
    fp = fopen("ur0:data/external/resolution.bin", "rb");
    fread(&mode, sizeof(mode),1,fp);
    fclose(fp);
    switch (mode)
    {
        case VITA_INVALID_WINDOW:
            return "Disabled";
            break;
        case VITA_WINDOW_960X544:
            return "960X544";
            break;
        case VITA_WINDOW_720X408:
            return "720X408";
            break;
        case VITA_WINDOW_640X368:
            return "640X368";
            break;
        case VITA_WINDOW_480X272:
            return "480X272";
            break;
        case VITA_WINDOW_1280X720:
            return "1280X720";
            break;
        case VITA_WINDOW_1920X1080:
            return "1920x1080";
            break;
    }
}

void setResolutionConfig()
{
    psvDebugScreenClear(0);
    createDirectory("ur0:data/external");

    psvDebugScreenClear(0);
    FILE *fp;

    int res;
	char displayModeUtf8[6];
	uint16_t displayMode[6];

	SceImeDialogParam param;
	memset(&param, 0, sizeof(param)); 
	sceImeDialogParamInit(&param);
	
	param.title = u"Enter Resolution Mode (0-6)";
	param.maxTextLength = 1;
	param.initialText = u"";
	param.inputTextBuffer = displayMode;
	param.type = SCE_IME_TYPE_NUMBER;
	res = CallImeDialog(&param);
	utf16_to_utf8((const uint16_t *)&displayMode, (uint8_t *)&displayModeUtf8);
    psvDebugScreenClear(0);
    psvDebugScreenSet();

    if(res < 0){ 
		ERROR("Error : CallImeDialog failed: %x\n", res);
        LOG("Press Any Button to Go Back\n");
        get_key(1);
        get_key(0);
		return;
	}

    int mode = atoi(displayModeUtf8);
    LOG("Mode: %d\nCustom Resolution: ", mode);
    switch (mode)
    {
        case VITA_INVALID_WINDOW:
            WARN("Disabled\n");
            break;
        case VITA_WINDOW_960X544:
            LOG("960X544\n");
            break;
        case VITA_WINDOW_720X408:
            LOG("720X408\n");
            break;
        case VITA_WINDOW_640X368:
            LOG("640X368\n");
            break;
        case VITA_WINDOW_480X272:
            LOG("480X272\n");
            break;
        case VITA_WINDOW_1280X720:
            LOG("1280X720\n");
            if (!vshSblAimgrIsDolce()) {
                WARN("REQUIRES SHARPSCALE ON THE VITA!\n");
            }
            break;
        case VITA_WINDOW_1920X1080:
            LOG("1920x1080\n");
            if (!vshSblAimgrIsDolce()) {
                WARN("REQUIRES SHARPSCALE ON THE VITA!\n");
            }
            break;
        default:
            psvDebugScreenClear(0);
            ERROR("Invalid Resolution Mode. Please Use a Number From 0-6.\n\n");
            LOG("Press Any Button to Go Back\n");
            get_key(1);
            get_key(0);
            return;
            break;
    }
    fp = fopen("ur0:data/external/resolution.bin", "wb");
    fwrite(&mode, sizeof(mode),1,fp);
    fclose(fp);

    SUCCESS("\nSet Forced Resolution Mode\n\n");
    LOG("Press Any Button to Go Back\n");
    get_key(1);
    get_key(0);
}

void creditMenu()
{   
    psvDebugScreenClear(0);
    LOG("Pigs In A Blanket Credits:\n\n")
    LOG("SonicMastr - Reversal/PIB Library/Shader Integration\n");
    LOG("dots-tb - Intitial Idea (PSM)/Reversal/Tester\n");
    LOG("GrapheneCt - Discovered Piglet/Reversal/Tester/Debugging\n");
    LOG("cuevavirus - Debugging\n");
    LOG("CreepNT - Debugging\n");
    LOG("Princess-of-Sleeping - Dump Tool/PrincessLog\n\n");
    WARN("Special Thanks:\n");
    LOG("xyzz - Learning how ShaccCg works\n");
    LOG("Zer0xFF and masterzorag - Piglet PS4 Reference\n\n");
    LOG("Press Circle to Go Back\n");
    get_key(1);
    while (1) {
        switch(get_key(0)) {
		case SCE_CTRL_CIRCLE:
            return;
			break;
        default:
            break;
	    }
    }
}

void install()
{
    psvDebugScreenClear(0);
    createDirectory("ur0:data/external");
    int shaccStat = saveFile(&_binary_resources_libshacccg_suprx_start, &_binary_resources_libshacccg_suprx_size, "ur0:data/external/libshacccg.suprx", "libshacccg.suprx");
    int pigStat = saveFile(&_binary_resources_libScePiglet_suprx_start, &_binary_resources_libScePiglet_suprx_size, "ur0:data/external/libScePiglet.suprx", "libScePiglet.suprx");
    if (pigStat)
    {
        ERROR("Failed to Install Pib. Please Try Again.\n");
    } else if (shaccStat) {
        WARN("Partially Installed PIB\n\n");
    } else {
        SUCCESS("Installed PIB!\n\n");
    }
    LOG("Press Any Button to Go Back\n");
    get_key(1);
    get_key(0);
}

void mainMenu()
{
    psvDebugScreenClear(0);
    int hasShacc = fileExists("libshacccg.suprx", "ur0:data/external");
    int hasPiglet = fileExists("libScePiglet.suprx", "ur0:data/external");
    LOG("Pigs In A Blanket Configuration Tool %.2f\nBy SonicMastr and CBPS\n\n", VERSION);
    LOG("libshacccg         - ");
    if (hasShacc)
    {
        SUCCESS("Installed\n");
    } else {
        ERROR("Not Installed\n");
    }
    LOG("libScePiglet       - ");
    if (hasPiglet)
    {
        SUCCESS("Installed\n");
    } else {
        ERROR("Not Installed\n");
    }

    LOG("PIB Install Status - ");
    if (hasPiglet)
    {
        if (hasShacc)
        {
            SUCCESS("Installed\n");
        } else {
            WARN("Partially Installed\n");
        }
    } else {
        ERROR("Not Installed\n");
    }
    LOG("Forced Resolution  - ");
    if (hasResolutionConfig())
    {
        LOG("%s\n\n", getResolutionConfig());
    } else {
        WARN("Not Configured\n\n");
    }

    if (hasPiglet)
    {
        LOG("CROSS: Re-install/Repair PIB\n");
    } else {
        LOG("CROSS: Install PIB\n");
    }
    LOG("SQUARE: Set Forced Resolution Mode\n");
    LOG("TRIANGLE: Credits\n");
    LOG("CIRCLE: Exit Application\n");

    get_key(1); 
    while(1){
        switch(get_key(0)) {
		case SCE_CTRL_CROSS:
			install();
            return;
			break;
        case SCE_CTRL_SQUARE:
            setResolutionConfig();
            return;
            break;
        case SCE_CTRL_TRIANGLE:
            creditMenu();
            return;
            break;
		case SCE_CTRL_CIRCLE:
            gxm_term();
			sceKernelExitProcess(0);
			break;
		default:
			break;
	    }
    }
}

int main()
{
    gxm_init();
    psvDebugScreenInit();
    psvDebugScreenClear(0);
    while (1) {
        mainMenu();
    }
    gxm_term();
    sceKernelExitProcess(0);
    return 0;
}