#include "main.h"
#include <stdio.h>
#include <string.h>

void configure_rtc_date(RTC_DateTypeDef *date)
{
	HAL_RTC_SetDate(&hrtc, date, RTC_FORMAT_BIN);
}
void configure_rtc_time(RTC_TimeTypeDef *time)
{
	time->TimeFormat = RTC_HOURFORMAT12_AM; /* Should take the time format from user. */
	HAL_RTC_SetTime(&hrtc, time, RTC_FORMAT_BIN);
}
void show_date_time(void)
{
	static char showtime[40];
	static char showdate[40];

	RTC_DateTypeDef rtc_date;
	RTC_TimeTypeDef rtc_time;

	static char *time = showtime;
	static char *date = showdate;

	memset(&rtc_date, 0, sizeof(rtc_date));
	memset(&rtc_time, 0, sizeof(rtc_time));

	/*Get the RTC Time */
	HAL_RTC_GetTime(&hrtc, &rtc_time, RTC_FORMAT_BIN);
	/* Get RTC current date */
    HAL_RTC_GetDate(&hrtc, &rtc_date, RTC_FORMAT_BIN);

    char *format = (rtc_time.TimeFormat == RTC_HOURFORMAT12_AM) ? "AM" : "PM";

    /* Display time Format : hh:mm:ss [AM/PM]*/
    sprintf((char*)showtime,"%s: \t%02d:%02d:%02d [%s]","\nCurrent Time&Date",rtc_time.Hours, rtc_time.Minutes,rtc_time.Seconds,format);
    xQueueSend(queue_print,&time,portMAX_DELAY);

    /* Display date Format : date-month-year */
    sprintf((char*)showdate,"\t%02d-%02d-%02d\n",rtc_date.Month, rtc_date.Date, 2000 + rtc_date.Year);
    xQueueSend(queue_print,&date,portMAX_DELAY);
}

int validate_rtc_information(RTC_TimeTypeDef *time , RTC_DateTypeDef *date)
{
	if(time){
		if( (time->Hours > 12) || (time->Minutes > 59) || (time->Seconds > 59) )
			return 1;
	}

	if(date){
		if( (date->Date > 31) || (date->WeekDay > 7) || (date->Year > 99) || (date->Month > 12) )
			return 1;
	}

	return 0;
}
