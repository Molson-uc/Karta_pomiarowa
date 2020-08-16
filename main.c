#include <atmel_start.h>



	bool convert=false;
	
	bool convert1=false;
	
	static void TIMER_0task2cb(const struct timer_task *const timer_task);
	static void TIMER_0task1cb(const struct timer_task *const timer_task);
	
//----------------------------------------ADC---------------------------------------//
	static void convert_cb_ADC_0(const struct adc_async_descriptor *const descr, const uint8_t channel)
	{
		convert=true;
	}


	void ADC0init(uint8_t chanel)
	{
		adc_async_register_callback(&ADC_0, chanel, ADC_ASYNC_CONVERT_CB, convert_cb_ADC_0);
		adc_async_enable_channel(&ADC_0,chanel);
		
		adc_async_start_conversion(&ADC_0);
	}
//------------------------------------Timer----------------------------------------//

static struct timer_task TIMER_0task1, TIMER_0task2;

/**
 * Example of using TIMER_0.
 */
uint32_t cycle;
static void TIMER_0task1cb(const struct timer_task *const timer_task)
{
	cycle++;
}
uint32_t cycle2;
static void TIMER_0task2cb(const struct timer_task *const timer_task)
{
	cycle2++;
}

void TIMER_0_configure(void)
{

	TIMER_0task1.interval = 100;
	TIMER_0task1.cb       = TIMER_0task1cb;
	TIMER_0task1.mode     = TIMER_TASK_REPEAT;
	TIMER_0task2.interval = 200;
	TIMER_0task2.cb       = TIMER_0task2cb;
	TIMER_0task2.mode    = TIMER_TASK_REPEAT;
	
	timer_add_task(&TIMER_0, &TIMER_0task1);
	timer_add_task(&TIMER_0, &TIMER_0task2);
	
	timer_start(&TIMER_0);
}

//-------------------------------------------external----------------------------------------//
uint16_t tc_old_c;
uint16_t tc_new_c;
uint16_t speed1;
uint16_t count1;
uint16_t speed_rad;
void encoder1()
{
	tc_old_c = tc_new_c;
	tc_new_c=cycle;
	count1=tc_new_c-tc_old_c;
	speed_rad=0.1745/count1;
	speed1=speed_rad*0.25;
	
}
uint16_t tc_old_c2;
uint16_t tc_new_c2;
uint16_t speed2;
uint16_t count2;
uint16_t speed_rad2;
void encoder2()
{
		tc_old_c2 = tc_new_c2;
		tc_new_c2=cycle2;
		count2=tc_new_c2-tc_old_c2;
		speed_rad2=0.1745/count2;
		speed2=speed_rad2*0.25;	
}

void EXTERNAL_IRQ_0(void)
{

	ext_irq_register(PIN_PB04, encoder1);
	ext_irq_register(PIN_PB05, encoder2);
}
uint8_t tab_pin_analog[4] = {0, 1, 6, 7};
uint8_t result[4];	
uint8_t wynik[4];

//------------------------------------------ADC_function------------------------------------------//
void adc_0_function()
{
	for (int i =0; i<=3;i++)
	{
		ADC0init(tab_pin_analog[i]);
	
		while(!convert){}
		wynik[i] = adc_async_read_channel(&ADC_0,tab_pin_analog[i],&result[i],2);
	
		adc_async_disable_channel(&ADC_0,tab_pin_analog[i]);
	}	
}
//------------------------------------------CAN0------------------------------------------//
void CAN_0_txcallback(struct can_async_descriptor *const descr)
{
	(void)descr;
}

struct can_message msg;
void CAN_0_initation(void)
{
	struct can_message msg;
	struct can_filter  filter;
	uint8_t            send_data[6];
	send_data[0] = (uint8_t)speed1;
	send_data[1] = (uint8_t)speed2;
	send_data[2] = (uint8_t)wynik[1];
	send_data[3] = (uint8_t)wynik[2];
	send_data[4] = (uint8_t)wynik[3];
	send_data[5] = (uint8_t)wynik[4];

	msg.id   = 0x45A;
	msg.type = CAN_TYPE_DATA;
	msg.data = send_data;
	msg.len  = 6;
	msg.fmt  = CAN_FMT_STDID;
	can_async_register_callback(&CAN_0, CAN_ASYNC_TX_CB, (FUNC_PTR)CAN_0_txcallback);
	can_async_enable(&CAN_0);
}

void CAN_0_read()
{
	can_async_write(&CAN_0, &msg);

	msg.id  = 0x100000A5;
	msg.fmt = CAN_FMT_EXTID;
	/**
	 * remote device should recieve message with ID=0x100000A5
	 */
	can_async_write(&CAN_0, &msg);
	
}


//------------------------------------------MAIN------------------------------------------//

int main(void)
{
	
	atmel_start_init();
	EXTERNAL_IRQ_0();
	TIMER_0_configure();
	CAN_0_initation();
	
	while (1) {
	adc_0_function();
	CAN_0_read();
		
	}
}
