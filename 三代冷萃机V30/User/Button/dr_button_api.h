#ifndef _dr_key_H
#define _dr_key_H

#include "dr_button_reg.h"
typedef enum
{
    KEY_None = 0,
    KEY_SINGLE_CLICK,
    KEY_DOUBLE_CLICK,
    KEY_LONG_RRESS_START,
    KEY_PRESS
} ClickEvent;
 
void DR_Key_Init(void); 

int Get_Key1(void);
int Get_Key2(void);
int Get_Key3(void);
int Get_Key4(void);

void Key_Tick(void);

#endif
