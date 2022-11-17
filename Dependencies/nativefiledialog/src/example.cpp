#ifdef USING_IMGUI
#include "nfd.h"
#include <stdio.h>
#include <stdlib.h>

void createDialog()
{
    nfdchar_t* outPath = NULL;
    nfdresult_t result = NFD_OpenDialog(NULL, NULL, &outPath);

    if (result == NFD_OKAY) {
        puts("Success!");
        puts(outPath);
        free(outPath);
    }
    else if (result == NFD_CANCEL) {
        puts("User pressed cancel.");
    }
    else {
        printf("Error: %s\n", NFD_GetError());
    }
}
#endif