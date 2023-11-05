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

// Botão power
#define BUT4_PIO PIOD
#define BUT4_PIO_ID ID_PIOD
#define BUT4_PIO_PIN 22
#define BUT4_PIO_PIN_MASK (1 << BUT4_PIO_PIN)


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


/************************************************************************/
/* recursos RTOS                                                        */
/************************************************************************/

/** Semaforo a ser usado pela task led */
SemaphoreHandle_t xSemaphoreBut; //semaforo botao placa
SemaphoreHandle_t xSemaphoreBut1;
SemaphoreHandle_t xSemaphoreBut2;
SemaphoreHandle_t xSemaphoreBut3;
SemaphoreHandle_t xSemaphoreBut4;

/** Queue for msg log send data */
QueueHandle_t xQueueLedFreq;
/************************************************************************/
/* prototypes                                                           */
/************************************************************************/

extern void vApplicationStackOverflowHook(xTaskHandle *pxTask,
signed char *pcTaskName);
extern void vApplicationIdleHook(void);
extern void vApplicationTickHook(void);
extern void vApplicationMallocFailedHook(void);
extern void xPortSysTickHandler(void);
void but_callback(void);
void but1_callback(void);
void but2_callback(void);
void but3_callback(void);
void but4_callback(void);
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
void but_callback(void) {
	printf("callback botao 0 \n");
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(xSemaphoreBut, &xHigherPriorityTaskWoken); //se foi liberado de uma ISR
}

void but1_callback(void) {
	printf("callback botao 1 \n");
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(xSemaphoreBut1, &xHigherPriorityTaskWoken); //se foi liberado de uma ISR
}

void but2_callback(void) {
	printf("callback botao 2 \n");
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(xSemaphoreBut2, &xHigherPriorityTaskWoken); //se foi liberado de uma ISR
}

void but3_callback(void) {
	printf("callback botao 3 \n");
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(xSemaphoreBut3, &xHigherPriorityTaskWoken); //se foi liberado de uma ISR
}

void but4_callback(void) {
	printf("callback botao 4 \n");
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(xSemaphoreBut4, &xHigherPriorityTaskWoken); //se foi liberado de uma ISR
}
/************************************************************************/
/* funcoes                                                              */
/************************************************************************/

void io_init(void) {

	// Ativa PIOs
	pmc_enable_periph_clk(LED_PIO_ID);
	pmc_enable_periph_clk(BUT_PIO_ID);
	pmc_enable_periph_clk(BUT1_PIO_ID);
	pmc_enable_periph_clk(BUT2_PIO_ID);
	pmc_enable_periph_clk(BUT3_PIO_ID);
	pmc_enable_periph_clk(BUT4_PIO_ID);
	// Configura Pinos
	pio_configure(LED_PIO, PIO_OUTPUT_0, LED_IDX_MASK, PIO_DEFAULT | PIO_DEBOUNCE);
	pio_configure(BUT_PIO, PIO_INPUT, BUT_PIO_PIN_MASK, PIO_PULLUP);
	pio_configure(BUT1_PIO, PIO_INPUT, BUT1_PIO_PIN_MASK, PIO_PULLUP);
	pio_configure(BUT2_PIO, PIO_INPUT, BUT2_PIO_PIN_MASK, PIO_PULLUP);
	pio_configure(BUT3_PIO, PIO_INPUT, BUT3_PIO_PIN_MASK, PIO_PULLUP);
	pio_configure(BUT4_PIO, PIO_INPUT, BUT4_PIO_PIN_MASK, PIO_PULLUP);
	
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
	
	pio_handler_set(BUT4_PIO,
					BUT4_PIO_ID,
					BUT4_PIO_PIN_MASK,
					PIO_IT_FALL_EDGE,
					but4_callback);		
							
	// Ativa interrupção e limpa primeira IRQ gerada na ativacao
	pio_enable_interrupt(BUT_PIO, BUT_PIO_PIN_MASK);
	pio_get_interrupt_status(BUT_PIO);
	
	pio_enable_interrupt(BUT1_PIO, BUT1_PIO_PIN_MASK);
	pio_get_interrupt_status(BUT1_PIO);
	
	pio_enable_interrupt(BUT2_PIO, BUT2_PIO_PIN_MASK);
	pio_get_interrupt_status(BUT2_PIO);
	
	pio_enable_interrupt(BUT3_PIO, BUT3_PIO_PIN_MASK);
	pio_get_interrupt_status(BUT3_PIO);
	
	pio_enable_interrupt(BUT4_PIO, BUT4_PIO_PIN_MASK);
	pio_get_interrupt_status(BUT4_PIO);
	
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
	
	NVIC_EnableIRQ(BUT4_PIO_ID);
	NVIC_SetPriority(BUT4_PIO_ID, 4); // Prioridade 4
	
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

void task_bluetooth(void) {
	printf("Task Bluetooth started \n");
	
	printf("Inicializando HC05 \n");
	config_usart0();
	hc05_init();

	// configura LEDs e Botões
	io_init();

	
	char eof = 'X';

	// Task não deve retornar.
	while(1) {
		char button = '0';
		// atualiza valor do botão VERDE
		if(xSemaphoreTake(xSemaphoreBut, 1)) {
			button |= 0b00000001;
			printf("verde");
		}
		
		
		// atualiza valor do botão VERMELHO
		if(xSemaphoreTake(xSemaphoreBut1, 1)) {
			button |= 0b00000010;
			printf("vermelho");
		}
		
		
		// atualiza valor do botão AMARELO
		if(xSemaphoreTake(xSemaphoreBut2, 1)) {
			button |= 0b00000100;
			printf("ama");
		}
		
		
		// atualiza valor do botão AZUL
		if(xSemaphoreTake(xSemaphoreBut3, 1)) {
			button |= 0b00001000;
			printf("az");
		}
		
		// atualiza valor do botão POWER
		if(xSemaphoreTake(xSemaphoreBut4, 1)) {
			button |= 0b00001010;
			printf("pow");
		}
	
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
		vTaskDelay(500/ portTICK_PERIOD_MS);
		
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
	
	xSemaphoreBut4 = xSemaphoreCreateBinary();
	if (xSemaphoreBut4 == NULL){
	printf("falha em criar o semaforo \n"); }
	


	/* Create task to make led blink */
	xTaskCreate(task_bluetooth, "BLT", TASK_BLUETOOTH_STACK_SIZE, NULL,	TASK_BLUETOOTH_STACK_PRIORITY, NULL);

	/* Start the scheduler. */
	vTaskStartScheduler();

	while(1){}

	/* Will only get here if there was insufficient memory to create the idle task. */
	return 0;

	}
