#include "main.h"
#include <string.h>
const char* inv_msg = "Invalid Option selected \n";

#define HH_CONFIG 0
#define MM_CONFIG 1
#define SS_CONFIG 2

#define DATE_CONFIG 4
#define MONTH_CONFIG 5
#define DAY_CONFIG 6
#define YEAR_CONFIG 7

uint8_t getnumber(uint8_t *p , int len);

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
void rtc_task(void *parameter)
{
	const char* msg_rtc1 = "========================\n"
							"|         RTC          |\n"
							"========================\n";

	const char* msg_rtc2 = "Configure Time            ----> 0\n"
							"Configure Date            ----> 1\n"
							"Enable reporting          ----> 2\n"
							"Exit                      ----> 3\n"
							"Enter your choice here : ";


	const char *msg_rtc_hh = "Enter hour(1-12):";
	const char *msg_rtc_mm = "Enter minutes(0-59):";
	const char *msg_rtc_ss = "Enter seconds(0-59):";

	const char *msg_rtc_dd  = "Enter date(1-31):";
	const char *msg_rtc_mo  ="Enter month(1-12):";
	const char *msg_rtc_dow  = "Enter day(1-7 sun:1):";
	const char *msg_rtc_yr  = "Enter year(0-99):";

	const char *msg_conf = "Configuration successful\n";
	const char *msg_rtc_report = "Enable time&date reporting(y/n)?: ";


	uint32_t cmd_addr;
	command_t *cmd;
    int menu_code;
    int rtc_state = HH_CONFIG;
    RTC_DateTypeDef date;
    RTC_TimeTypeDef time;

	while(1){
		/*TODO: Notify wait (wait till someone notifies) */
		xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);

		/*TODO : Print the menu and show current date and time information */
        xQueueSend(queue_print,&msg_rtc1,portMAX_DELAY);
        show_date_time();
        xQueueSend(queue_print,&msg_rtc2,portMAX_DELAY);

		while(current_state != sMainMenu){

			/*TODO: Wait for command notification (Notify wait) */
            xTaskNotifyWait(0, 0, &cmd_addr, portMAX_DELAY);
            cmd = (command_t*)cmd_addr;
			switch(current_state)
			{
				case sRtcMenu:{

					/*TODO: process RTC menu commands */
					if(cmd->len == 1)
					{
						menu_code = cmd->payload[0] - 48;
						switch(menu_code)
						{
						case 0:
							current_state = sRtcTimeConfig;
							xQueueSend(queue_print,&msg_rtc_hh,portMAX_DELAY);
							break;
						case 1:
							current_state= sRtcDateConfig;
							xQueueSend(queue_print,&msg_rtc_dd,portMAX_DELAY);
							break;
						case 2:
							current_state = sRtcReport;
							xQueueSend(queue_print,&msg_rtc_report,portMAX_DELAY);
							break;
						case 3:
							current_state = sMainMenu;
							break;
						default:
							current_state = sMainMenu;
							xQueueSend(queue_print,&inv_msg,portMAX_DELAY);
						}
					}
					else
					{
						current_state = sMainMenu;
						xQueueSend(queue_print,&inv_msg,portMAX_DELAY);
					}
					break;}

				case sRtcTimeConfig:{
					/*TODO : get hh, mm, ss infor and configure RTC */

					/*TODO: take care of invalid entries */
					switch(rtc_state)
					{
					case HH_CONFIG:
						rtc_state = MM_CONFIG;
						uint8_t hour = getnumber(cmd->payload, cmd->len);
						time.Hours = hour;
						xQueueSend(queue_print,&msg_rtc_mm,portMAX_DELAY );
						break;
					case MM_CONFIG:
						rtc_state = SS_CONFIG;
						uint8_t minutes = getnumber(cmd->payload, cmd->len);
						time.Minutes = minutes;
						xQueueSend(queue_print,&msg_rtc_ss,portMAX_DELAY );
						break;
					case SS_CONFIG:
						uint8_t seconds = getnumber(cmd->payload, cmd->len);
						time.Seconds = seconds;
						if(!validate_rtc_information(&time,NULL))
						{
							configure_rtc_time(&time);
							xQueueSend(queue_print,&msg_conf,portMAX_DELAY );
							show_date_time();
						}
						else
							xQueueSend(queue_print,&inv_msg,portMAX_DELAY );

						current_state = sMainMenu;
						rtc_state = 0;
						break;
					default:
						current_state = sMainMenu;
						xQueueSend(queue_print,&msg_rtc2,portMAX_DELAY );
						break;
					}
					break;}

				case sRtcDateConfig:{


					/*TODO : get date, month, day , year info and configure RTC */
					rtc_state = DATE_CONFIG;

					/*TODO: take care of invalid entries */
					switch(rtc_state)
					{
					case DATE_CONFIG:
						rtc_state = MONTH_CONFIG;
						uint8_t dd = getnumber(cmd->payload, cmd->len);
						date.Date = dd;
						xQueueSend(queue_print,&msg_rtc_mo,portMAX_DELAY );
						break;
					case MONTH_CONFIG:
						rtc_state = DAY_CONFIG;
						uint8_t month = getnumber(cmd->payload, cmd->len);
						date.Month = month;
						xQueueSend(queue_print,&msg_rtc_dow,portMAX_DELAY );
						break;
					case DAY_CONFIG:
						rtc_state = YEAR_CONFIG;
						uint8_t day = getnumber(cmd->payload, cmd->len);
						date.WeekDay = day;
						xQueueSend(queue_print,&msg_rtc_yr,portMAX_DELAY );
						break;
					case YEAR_CONFIG:
						rtc_state = 0;
						uint8_t year = getnumber(cmd->payload, cmd->len);
						date.Year = year;
						if(!validate_rtc_information(NULL,&date))
						{
							configure_rtc_date(&date);
							xQueueSend(queue_print,&msg_conf,portMAX_DELAY );
							show_date_time();
						}
						else
							xQueueSend(queue_print,&inv_msg,portMAX_DELAY );

						current_state = sMainMenu;
						rtc_state = 0;
						break;
					default:
						current_state = sMainMenu;
						xQueueSend(queue_print,&msg_rtc2,portMAX_DELAY );
						break;

					break;}

				case sRtcReport:{
					/*TODO: enable or disable RTC current time reporting over ITM printf */
					break;}

			}// switch end

		} //while end

		   /*TODO : Notify menu task */
		current_state = sMainMenu;

		}//while super loop end
}
}
uint8_t getnumber(uint8_t *p , int len)
{

	int value ;

	if(len > 1)
	   value =  ( ((p[0]-48) * 10) + (p[1] - 48) );
	else
		value = p[0] - 48;

	return value;

}

