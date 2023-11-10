/************************************************************************
* 5 semestre - Eng. da Computao - Insper
*
* 2021 - Exemplo com HC05 com RTOS
*
*/

#include <asf.h>
#include "conf_board.h"
#include <string.h>

/************************************************************************/
/* defines                                                              */
/************************************************************************/

// LEDs
#define LED_PIO      PIOC
#define LED_PIO_ID   ID_PIOC
#define LED_IDX      8
#define LED_IDX_MASK (1 << LED_IDX)

// Botão verde
#define BUT_PIO PIOA
#define BUT_PIO_ID ID_PIOA
#define BUT_PIO_PIN 6
#define BUT_PIO_PIN_MASK (1 << BUT_PIO_PIN)

// Botão vermelho
#define BUT1_PIO PIOD
#define BUT1_PIO_ID ID_PIOD
#define BUT1_PIO_PIN 25
#define BUT1_PIO_PIN_MASK (1 << BUT1_PIO_PIN)

// Botão amarelo
#define BUT2_PIO PIOD
#define BUT2_PIO_ID ID_PIOD
#define BUT2_PIO_PIN 24
#define BUT2_PIO_PIN_MASK (1 << BUT2_PIO_PIN)

// Botão azul
#define BUT3_PIO PIOA
#define BUT3_PIO_ID ID_PIOA
#define BUT3_PIO_PIN 24
#define BUT3_PIO_PIN_MASK (1 << BUT3_PIO_PIN)


//AFEC

#define AFEC_POT AFEC0
#define AFEC_POT_ID ID_AFEC0
#define AFEC_POT_CHANNEL 0 // Canal do pino PD30

// usart (bluetooth ou serial)
// Descomente para enviar dados
// pela serial debug

//#define DEBUG_SERIAL

#ifdef DEBUG_SERIAL
#define USART_COM USART1
#define USART_COM_ID ID_USART1
#else
#define USART_COM USART0
#define USART_COM_ID ID_USART0
#endif

/************************************************************************/
/* RTOS                                                                 */
/************************************************************************/

#define TASK_BLUETOOTH_STACK_SIZE            (4096/sizeof(portSTACK_TYPE))
#define TASK_BLUETOOTH_STACK_PRIORITY        (tskIDLE_PRIORITY)

#define TASK_BUT_STACK_SIZE (4096 / sizeof(portSTACK_TYPE))
#define TASK_BUT_STACK_PRIORITY (tskIDLE_PRIORITY)

#define TASK_ADC_STACK_SIZE (1024 * 10 / sizeof(portSTACK_TYPE))
#define TASK_ADC_STACK_PRIORITY (tskIDLE_PRIORITY)

/************************************************************************/
/* recursos RTOS                                                        */
/************************************************************************/

/** Semaforo a ser usado pela task led */
SemaphoreHandle_t xSemaphoreBut; //semaforo botao placa
SemaphoreHandle_t xSemaphoreBut1;
SemaphoreHandle_t xSemaphoreBut2;
SemaphoreHandle_t xSemaphoreBut3;

TimerHandle_t xTimer;

/** Queue for msg log send data */
QueueHandle_t xQueueLedFreq;
/** Queue for msg log send data */

QueueHandle_t xQueueADC; /** fila para enviar o afec


QueueHandle_t xQueuePot; /** Criando a fila para receber as informações*/
QueueHandle_t xQueuePot; /** Criando a fila para receber as informações*/

typedef struct {
	uint value;
} adcData;

/************************************************************************/
/* prototypes                                                           */
/************************************************************************/

extern void vApplicationStackOverflowHook(xTaskHandle *pxTask,
signed char *pcTaskName);
extern void vApplicationIdleHook(void);
extern void vApplicationTickHook(void);
extern void vApplicationMallocFailedHook(void);
extern void xPortSysTickHandler(void);
static void config_AFEC_pot(Afec *afec, uint32_t afec_id, uint32_t afec_channel,
afec_callback_t callback);
static void AFEC_pot0_callback(void);
static void configure_console(void);

/************************************************************************/
/* constants                                                            */
/************************************************************************/

/************************************************************************/
/* variaveis globais                                                    */
/************************************************************************/

/************************************************************************/
/* RTOS application HOOK                                                */
/************************************************************************/

/* Called if stack overflow during execution */
extern void vApplicationStackOverflowHook(xTaskHandle *pxTask,
signed char *pcTaskName) {
	printf("stack overflow %x %s\r\n", pxTask, (portCHAR *)pcTaskName);
	/* If the parameters have been corrupted then inspect pxCurrentTCB to
	* identify which task has overflowed its stack.
	*/
	for (;;) {
	}
}

/* This function is called by FreeRTOS idle task */
extern void vApplicationIdleHook(void) {
	pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);
}

/* This function is called by FreeRTOS each tick */
extern void vApplicationTickHook(void) { }

extern void vApplicationMallocFailedHook(void) {
	/* Called if a call to pvPortMalloc() fails because there is insufficient
	free memory available in the FreeRTOS heap.  pvPortMalloc() is called
	internally by FreeRTOS API functions that create tasks, queues, software
	timers, and semaphores.  The size of the FreeRTOS heap is set by the
	configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */

	/* Force an assert. */
	configASSERT( ( volatile void * ) NULL );
}

/************************************************************************/
/* handlers / callbacks                                                 */
/************************************************************************/
// Global para armazenar o último valor do ADC
static void AFEC_pot_callback(void) {
	
	adcData adc;
	adc.value = afec_channel_get_value(AFEC_POT, AFEC_POT_CHANNEL);
	
	BaseType_t xHigherPriorityTaskWoken = pdTRUE;
	xQueueSendFromISR(xQueueADC, &adc, &xHigherPriorityTaskWoken); /* Coloca dados na fila se for de uma ISR*/
}

void vTimerCallback(TimerHandle_t xTimer) {
	/* Selecina canal e inicializa conversão */
	afec_channel_enable(AFEC_POT, AFEC_POT_CHANNEL);
	afec_start_software_conversion(AFEC_POT);
}

void but_callback(void) {
	
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(xSemaphoreBut, &xHigherPriorityTaskWoken); //se foi liberado de uma ISR
}

void but1_callback(void) {
	
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(xSemaphoreBut1, &xHigherPriorityTaskWoken); //se foi liberado de uma ISR
}

void but2_callback(void) {
	
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(xSemaphoreBut2, &xHigherPriorityTaskWoken); //se foi liberado de uma ISR
}

void but3_callback(void) {
	
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(xSemaphoreBut3, &xHigherPriorityTaskWoken); //se foi liberado de uma ISR
}


/************************************************************************/
/* funcoes                                                              */
/************************************************************************/
// _pio_set
void _pio_set(Pio *p_pio, const uint32_t ul_mask)
{
	// setando o registro para ficar high
	p_pio ->PIO_SODR = ul_mask;
}

//_pio_clear
void _pio_clear(Pio *p_pio, const uint32_t ul_mask)
{
	// setando o registro para limpar
	p_pio ->PIO_CODR = ul_mask;
}


void io_init(void) {

	// Ativa PIOs
	pmc_enable_periph_clk(LED_PIO_ID);
	pmc_enable_periph_clk(BUT_PIO_ID);
	pmc_enable_periph_clk(BUT1_PIO_ID);
	pmc_enable_periph_clk(BUT2_PIO_ID);
	pmc_enable_periph_clk(BUT3_PIO_ID);
	
	// Configura Pinos
	pio_configure(LED_PIO, PIO_OUTPUT_0, LED_IDX_MASK, PIO_DEFAULT | PIO_DEBOUNCE);
	pio_configure(BUT_PIO, PIO_INPUT, BUT_PIO_PIN_MASK, PIO_PULLUP);
	pio_configure(BUT1_PIO, PIO_INPUT, BUT1_PIO_PIN_MASK, PIO_PULLUP);
	pio_configure(BUT2_PIO, PIO_INPUT, BUT2_PIO_PIN_MASK, PIO_PULLUP);
	pio_configure(BUT3_PIO, PIO_INPUT, BUT3_PIO_PIN_MASK, PIO_PULLUP);
	// Configura interrupção no pino referente ao botao e associa
	// função de callback caso uma interrupção for gerada
	// a função de callback é a: but_callback()
	pio_handler_set(BUT_PIO,
					BUT_PIO_ID,
					BUT_PIO_PIN_MASK,
					PIO_IT_FALL_EDGE,
					but_callback);
					
	pio_handler_set(BUT1_PIO,
					BUT1_PIO_ID,
					BUT1_PIO_PIN_MASK,
					PIO_IT_FALL_EDGE,
					but1_callback);
	
	pio_handler_set(BUT2_PIO,
					BUT2_PIO_ID,
					BUT2_PIO_PIN_MASK,
					PIO_IT_FALL_EDGE,
					but2_callback);
	
	pio_handler_set(BUT3_PIO,
					BUT3_PIO_ID,
					BUT3_PIO_PIN_MASK,
					PIO_IT_FALL_EDGE,
					but3_callback);
	
		
							
	// Ativa interrupção e limpa primeira IRQ gerada na ativacao
	pio_enable_interrupt(BUT_PIO, BUT_PIO_PIN_MASK);
	pio_get_interrupt_status(BUT_PIO);
	
	pio_enable_interrupt(BUT1_PIO, BUT1_PIO_PIN_MASK);
	pio_get_interrupt_status(BUT1_PIO);
	
	pio_enable_interrupt(BUT2_PIO, BUT2_PIO_PIN_MASK);
	pio_get_interrupt_status(BUT2_PIO);
	
	pio_enable_interrupt(BUT3_PIO, BUT3_PIO_PIN_MASK);
	pio_get_interrupt_status(BUT3_PIO);
	

	
	// Configura NVIC para receber interrupcoes do PIO do botao
	// com prioridade 4 (quanto mais próximo de 0 maior)
	NVIC_EnableIRQ(BUT_PIO_ID);
	NVIC_SetPriority(BUT_PIO_ID, 4); // Prioridade 4
	
	NVIC_EnableIRQ(BUT1_PIO_ID);
	NVIC_SetPriority(BUT1_PIO_ID, 4); // Prioridade 4
	
	NVIC_EnableIRQ(BUT2_PIO_ID);
	NVIC_SetPriority(BUT2_PIO_ID, 4); // Prioridade 4
	
	NVIC_EnableIRQ(BUT3_PIO_ID);
	NVIC_SetPriority(BUT3_PIO_ID, 4); // Prioridade 4
	

}


static void config_AFEC_pot(Afec *afec, uint32_t afec_id, uint32_t afec_channel,
                            afec_callback_t callback) {
  /*************************************
   * Ativa e configura AFEC
   *************************************/
  /* Ativa AFEC - 0 */
  afec_enable(afec);

  /* struct de configuracao do AFEC */
  struct afec_config afec_cfg;

  /* Carrega parametros padrao */
  afec_get_config_defaults(&afec_cfg);

  /* Configura AFEC */
  afec_init(afec, &afec_cfg);

  /* Configura trigger por software */
  afec_set_trigger(afec, AFEC_TRIG_SW);

  /*** Configuracao específica do canal AFEC ***/
  struct afec_ch_config afec_ch_cfg;
  afec_ch_get_config_defaults(&afec_ch_cfg);
  afec_ch_cfg.gain = AFEC_GAINVALUE_0;
  afec_ch_set_config(afec, afec_channel, &afec_ch_cfg);

  /*
  * Calibracao:
  * Because the internal ADC offset is 0x200, it should cancel it and shift
  down to 0.
  */
  afec_channel_set_analog_offset(afec, afec_channel, 0x200);

  /***  Configura sensor de temperatura ***/
  struct afec_temp_sensor_config afec_temp_sensor_cfg;

  afec_temp_sensor_get_config_defaults(&afec_temp_sensor_cfg);
  afec_temp_sensor_set_config(afec, &afec_temp_sensor_cfg);

  /* configura IRQ */
  afec_set_callback(afec, afec_channel, callback, 1);
  NVIC_SetPriority(afec_id, 4);
  NVIC_EnableIRQ(afec_id);
}

static void configure_console(void) {
	const usart_serial_options_t uart_serial_options = {
		.baudrate = CONF_UART_BAUDRATE,
		#if (defined CONF_UART_CHAR_LENGTH)
		.charlength = CONF_UART_CHAR_LENGTH,
		#endif
		.paritytype = CONF_UART_PARITY,
		#if (defined CONF_UART_STOP_BITS)
		.stopbits = CONF_UART_STOP_BITS,
		#endif
	};

	/* Configure console UART. */
	stdio_serial_init(CONF_UART, &uart_serial_options);

	/* Specify that stdout should not be buffered. */
	#if defined(__GNUC__)
	setbuf(stdout, NULL);
	#else
	/* Already the case in IAR's Normal DLIB default configuration: printf()
	* emits one character at a time.
	*/
	#endif
}

uint32_t usart_puts(uint8_t *pstring) {
	uint32_t i ;

	while(*(pstring + i))
	if(uart_is_tx_empty(USART_COM))
	usart_serial_putchar(USART_COM, *(pstring+i++));
}

void usart_put_string(Usart *usart, char str[]) {
	usart_serial_write_packet(usart, str, strlen(str));
}

int usart_get_string(Usart *usart, char buffer[], int bufferlen, uint timeout_ms) {
	uint timecounter = timeout_ms;
	uint32_t rx;
	uint32_t counter = 0;

	while( (timecounter > 0) && (counter < bufferlen - 1)) {
		if(usart_read(usart, &rx) == 0) {
			buffer[counter++] = rx;
		}
		else{
			timecounter--;
			vTaskDelay(1);
		}
	}
	buffer[counter] = 0x00;
	return counter;
}

void usart_send_command(Usart *usart, char buffer_rx[], int bufferlen,
char buffer_tx[], int timeout) {
	usart_put_string(usart, buffer_tx);
	usart_get_string(usart, buffer_rx, bufferlen, timeout);
}

void config_usart0(void) {
	sysclk_enable_peripheral_clock(ID_USART0);
	usart_serial_options_t config;
	config.baudrate = 9600;
	config.charlength = US_MR_CHRL_8_BIT;
	config.paritytype = US_MR_PAR_NO;
	config.stopbits = false;
	usart_serial_init(USART0, &config);
	usart_enable_tx(USART0);
	usart_enable_rx(USART0);

	// RX - PB0  TX - PB1
	pio_configure(PIOB, PIO_PERIPH_C, (1 << 0), PIO_DEFAULT);
	pio_configure(PIOB, PIO_PERIPH_C, (1 << 1), PIO_DEFAULT);
}

int hc05_init(void) {
	char buffer_rx[128];
	usart_send_command(USART_COM, buffer_rx, 1000, "AT", 100);
	vTaskDelay( 500 / portTICK_PERIOD_MS);
	usart_send_command(USART_COM, buffer_rx, 1000, "AT", 100);
	vTaskDelay( 500 / portTICK_PERIOD_MS);
	usart_send_command(USART_COM, buffer_rx, 1000, "AT+NAMEcearaMor", 100);
	vTaskDelay( 500 / portTICK_PERIOD_MS);
	usart_send_command(USART_COM, buffer_rx, 1000, "AT", 100);
	vTaskDelay( 500 / portTICK_PERIOD_MS);
	usart_send_command(USART_COM, buffer_rx, 1000, "AT+PIN0000", 100);
}

/************************************************************************/
/* TASKS                                                                */
/************************************************************************/



static void task_adc(void *pvParameters) {

  // configura ADC e TC para controlar a leitura
  config_AFEC_pot(AFEC_POT, AFEC_POT_ID, AFEC_POT_CHANNEL, AFEC_pot_callback);

  xTimer = xTimerCreate(/* Just a text name, not used by the RTOS
                        kernel. */
                        "Timer",
                        /* The timer period in ticks, must be
                        greater than 0. */
                        100,
                        /* The timers will auto-reload themselves
                        when they expire. */
                        pdTRUE,
                        /* The ID is used to store a count of the
                        number of times the timer has expired, which
                        is initialised to 0. */
                        (void *)0,
                        /* Timer callback */
                        vTimerCallback);
  xTimerStart(xTimer, 0);

  // Onde ficam os dados da fila
  int media;
  double final;

  while (1) {
    if (xQueueReceive(xQueueADC, &(media), 1000)) {
     
	  final = (double)media/4080;

	  xQueueSend(xQueuePot,&final,100);
    } else {
      printf("Nao chegou um novo dado em 1 segundo");
    }
  }
}

void task_bluetooth(void) {
	
	config_usart0();
	hc05_init();

	// configura LEDs e Botões
	io_init();

	
	char eof = 'X';
	
	char pack[128];
	double final;
	char x;
	bool handshake = false; 
	
	while(!handshake){
		if(!usart_read(USART_COM, &x)){
			if(x == 'z'){
				printf("Handshake ok");
				for(int i =0 ; i <=20 ; i++){
					_pio_set(LED_PIO, LED_IDX_MASK);
					vTaskDelay(5);
					_pio_clear(LED_PIO, LED_IDX_MASK);
				}
				handshake = true;
			}
			
		}else{
			printf("Enviando handshake");
			usart_write(USART_COM, 'z');
			vTaskDelay(1000/portTICK_PERIOD_MS);
		}
	}
	
	// Task não deve retornar.
	while(1) {
		
		if(xQueueReceive(xQueuePot, &final, 100)){
			// Concatena as strings de início, o valor e a string de fim
			sprintf(pack, "S%.1fZ", final);

			// Imprime o pacote para depuração
			

			
		}
		
		char button = '0';
		// atualiza valor do botão VERDE
		if(xSemaphoreTake(xSemaphoreBut, 1)) {
			button |= 0b00000001;
			
		}
		
		
		// atualiza valor do botão VERMELHO
		if(xSemaphoreTake(xSemaphoreBut1, 1)) {
			button |= 0b00000010;
			
		}
		
		
		// atualiza valor do botão AMARELO
		if(xSemaphoreTake(xSemaphoreBut2, 1)) {
			button |= 0b00000100;
			
		}
		
		
		// atualiza valor do botão AZUL
		if(xSemaphoreTake(xSemaphoreBut3, 1)) {
			button |= 0b00001000;
			
			
		}
		

		
		// Envia o pacote pela USART
		usart_put_string(USART_COM, pack);
		
		
		// envia status botão
		while(!usart_is_tx_ready(USART_COM)) {
			vTaskDelay(1 / portTICK_PERIOD_MS);
		}
		usart_write(USART_COM, button);
		
		// envia status botão
		while(!usart_is_tx_ready(USART_COM)) {
			vTaskDelay(1 / portTICK_PERIOD_MS);
		}
		usart_write(USART_COM, button);
		
		// envia fim de pacote
		while(!usart_is_tx_ready(USART_COM)) {
			vTaskDelay(1 / portTICK_PERIOD_MS);
		}
		usart_write(USART_COM, eof);

		// dorme por 500 ms
		
		vTaskDelay(5/ portTICK_PERIOD_MS);
		
	}
}



/************************************************************************/
/* main                                                                 */
/************************************************************************/

int main(void) {
	/* Initialize the SAM system */
	sysclk_init();
	board_init();

	configure_console();
	
	/* Attempt to create a semaphore. */
	xSemaphoreBut = xSemaphoreCreateBinary();
	if (xSemaphoreBut == NULL){
	printf("falha em criar o semaforo \n"); }
	
	xSemaphoreBut1 = xSemaphoreCreateBinary();
	if (xSemaphoreBut1 == NULL){
	printf("falha em criar o semaforo \n"); }
	
	xSemaphoreBut2 = xSemaphoreCreateBinary();
	if (xSemaphoreBut2 == NULL){
	printf("falha em criar o semaforo \n"); }
	
	xSemaphoreBut3 = xSemaphoreCreateBinary();
	if (xSemaphoreBut3 == NULL){
	printf("falha em criar o semaforo \n"); }
	


	
	  xQueueADC = xQueueCreate(100, sizeof(adcData));
	  xQueuePot = xQueueCreate(100, sizeof(double));
	  
	  if (xQueueADC == NULL){
	  printf("falha em criar a queue xQueueADC \n");}
	


	/* Create task to make led blink */
	xTaskCreate(task_bluetooth, "BLT", TASK_BLUETOOTH_STACK_SIZE, NULL,	TASK_BLUETOOTH_STACK_PRIORITY, NULL);
	 xTaskCreate(task_adc, "ADC", TASK_BUT_STACK_SIZE, NULL, TASK_BUT_STACK_PRIORITY, NULL);


	/* Start the scheduler. */
	vTaskStartScheduler();

	while(1){}

	/* Will only get here if there was insufficient memory to create the idle task. */
	return 0;

	}