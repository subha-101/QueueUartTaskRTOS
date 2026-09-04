#include "main.h"
#include <string.h>
const char* inv_msg = "Invalid Option selected \n";


void menu_task(void *Parameters)
{
	uint32_t command_addr;
	command_t* cmd;
	int option;

	const char* msg_menu = "========================\n"
							"|      Menu     |\n"
						   "========================\n"
							  "LED effect  ----> 0\n"
							  "Date and Time---> 1\n"
			                  "Exit -----------> 2\n"
			                  "Enter your choise here  :\n";


	while(1)
    {
		//Prints the message for user options or commands on serial monitor.
		xQueueSend(queue_print, &msg_menu, portMAX_DELAY);
		//Waits for the notification with the command address to be passed by the notification.
		xTaskNotifyWait(0, 0, &command_addr, portMAX_DELAY);

		cmd = (command_t*)command_addr;

		if(cmd->len == 1)
		{
			option = cmd->payload[0] - 48;
			switch(option)
			{
			case 0:
				current_state = sLedEffect;
				xTaskNotify(handle_led_task,0,eNoAction);
				break;
			case 1:
				current_state = sRtcMenu;
				xTaskNotify(handle_rtc_task,0,eNoAction);
				break;
			case 2:
				/* Implement Exit*/
				break;
			default:
				//invalid entry.
				xQueueSend(queue_print, &inv_msg, portMAX_DELAY);
				continue;
			}
		}
		else
		{
			//invalid entry.
			xQueueSend(queue_print, &inv_msg, portMAX_DELAY);
			continue;
		}
		//Waits for the notification with the command address to be passed by the notification.
		xTaskNotifyWait(0, 0, &command_addr, portMAX_DELAY);
     }/*end of while*/
}
void cmd_handler_task(void *Parameters)
{
	BaseType_t ret;
	command_t cmd;
	  while(1)
	  {
		  ret = xTaskNotifyWait(0, 0, NULL,portMAX_DELAY);
		  if(ret == pdTRUE)
		  {
			  process_command(&cmd);
		  }
	  }
}
void process_command(command_t *cmd)
{
	extract_command(cmd);

	switch(current_state)
	{
	case sMainMenu:
		/* Notify the task with the command */
		xTaskNotify(handle_menu_task,(uint32_t)cmd, eSetValueWithOverwrite);
		break;
	case sLedEffect:
		/* Notify the task with the command */
		xTaskNotify(handle_led_task,(uint32_t)cmd, eSetValueWithOverwrite);
		break;
	case sRtcMenu:
	case sRtcTimeConfig:
	case sRtcDateConfig:
	case sRtcReport:
		/* Notify the task with the command */
		xTaskNotify(handle_rtc_task,(uint32_t)cmd, eSetValueWithOverwrite);
		break;

	}
}

int extract_command(command_t *cmd)
{
	uint8_t item;
	BaseType_t status;
	status = uxQueueMessagesWaiting( queue_data );
	if(! status) return -1;

	uint8_t i = 0;

	do {
		status = xQueueReceive(queue_data, &item,0);
		if(status == pdTRUE) cmd->payload[i++] = item;
	}while(item != '\n');

	cmd->payload[i-1] = '\0';
	cmd->len = i-1; /* Saving length of the command excluding null character */

	return 0;
}
void print_task(void *Parameters)
{
	uint8_t* msg;
	while(1)
	{
		xQueueReceive(queue_print, &msg, portMAX_DELAY);
		HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen((char*)msg), HAL_MAX_DELAY);
	}
}

void led_task(void *param)
{
	uint32_t cmd_addr;
	command_t *cmd;
	const char* msg_led = "========================\n"
						  "|      LED Effect     |\n"
						  "========================\n"
						  "(none,e1,e2,e3,e4)\n"
						  "Enter your choice here : ";

	while(1){
		/*TODO: Wait for notification (Notify wait) */
        xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);
		/*TODO: Print LED menu */
        xQueueSend(queue_print,&msg_led,portMAX_DELAY);
		/*TODO: wait for LED command (Notify wait) */
        xTaskNotifyWait(0, 0,&cmd_addr, portMAX_DELAY);
        cmd =(command_t*) cmd_addr;
		if(cmd->len <= 4)
		{
			if(! strcmp((char*)cmd->payload,"none"))
				led_effect_stop();
			else if (! strcmp((char*)cmd->payload,"e1"))
				led_effect(1);
			else if (! strcmp((char*)cmd->payload,"e2"))
				led_effect(2);
			else if (! strcmp((char*)cmd->payload,"e3"))
				led_effect(3);
			else if (! strcmp((char*)cmd->payload,"e4"))
				led_effect(4);
			else
			{
				/*TODO: print invalid message */
				xQueueSend(queue_print, &inv_msg, portMAX_DELAY);
			}

		}
		else
		{
			/*TODO: print invalid message */
			xQueueSend(queue_print, &inv_msg, portMAX_DELAY);
		}

		/*TODO : update state variable */
		current_state = sMainMenu;

		/*TODO : Notify menu task */
		xTaskNotify(handle_menu_task,0,eNoAction);

	}
}
void rtc_task(void *Parameters)
{
	while(1)
	{
	}
}
