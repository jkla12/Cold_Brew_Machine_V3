#include "gd32f30x.h"
#include "systick.h"
#include <stdio.h>
#include "main.h"
#include "uart.h"
#include <stdio.h>
#include <string.h>

#include "timer.h"

#include "work.h"

#ifdef 	LOG_TAG
#undef	LOG_TAG
#endif
#define LOG_TAG    "main"

/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{
    hw_init();
    init_task_queue();
    task_init();    
    while (1)
    {
        execute_state_machine();
        scheduler();


        if(Timer.flag == 1) 
        {
            Timer.flag = 0;
        }
        if(Timer._10msFlag == 1)
        {
            Timer._10msFlag = 0;
            
        }
        if (Timer._100msFlag == 1)
        {
            Timer._100msFlag = 0;
			
        }
        if (Timer._1sFlag == 1)
        {
            Timer._1sFlag = 0;
			
        }
    }
}



