#define F_CPU 20000000UL

#define LCD_HI_DDR DDRD
#define LCD_HI_PORT PORTD

#define LCD_LO_DDR DDRD
#define LCD_LO_PORT PORTD

#define LCD_RS_DDR DDRB
#define LCD_RS_PORT PORTB
#define LCD_RS_PIN 3

#define LCD_WR_DDR DDRB
#define LCD_WR_PORT PORTB
#define LCD_WR_PIN 4

#define LCD_RD_DDR DDRA
#define LCD_RD_PORT PORTA
#define LCD_RD_PIN 3

/*#define LCD_CS_DDR DDRC
#define LCD_CS_PORT PORTC
#define LCD_CS_PIN 0*/

#define LCD_REST_DDR DDRC
#define LCD_REST_PORT PORTC
#define LCD_REST_PIN 1

//#define LCD_SET_CS()    LCD_CS_PORT |= 1 << LCD_CS_PIN
//#define LCD_CLR_CS()    LCD_CS_PORT &= ~(1 << LCD_CS_PIN)

#define LCD_SET_RS()    LCD_RS_PORT |= 1 << LCD_RS_PIN
#define LCD_CLR_RS()    LCD_RS_PORT &= ~(1 << LCD_RS_PIN)

#define LCD_SET_WR()    LCD_WR_PORT |= 1 << LCD_WR_PIN
#define LCD_CLR_WR()    LCD_WR_PORT &= ~(1 << LCD_WR_PIN)
#define PULSE_WR_LOW()  LCD_CLR_WR(); asm volatile ("nop");  LCD_SET_WR();

#define LCD_SET_RD()    LCD_RD_PORT |= 1 << LCD_RD_PIN
#define LCD_CLR_RD()    LCD_RD_PORT &= ~(1 << LCD_RD_PIN)

#define LCD_SET_REST()  LCD_REST_PORT |= 1 << LCD_REST_PIN
#define LCD_CLR_REST()  LCD_REST_PORT &= ~(1 << LCD_REST_PIN)

// контроллеры
//#define SSD1289 1
#define ILI9328 2

// ориентация
#define PORTRAIT 	0
#define LANDSCAPE 1

//===========================================================================================
#define TRANSFER_MODE_16	0
#define TRANSFER_MODE_8		1
#define TRANSFER_MODE_OTHER	2
//===========================================================================================

#define X_begin 0 // координата первой точки по оси Х
#define Y_begin 0 // координата первой точки по оси Y

//===========================================================================================
// Цвета
//========================================================================================================
#define VGA_BLACK		      0x0000
#define VGA_WHITE		      0xFFFF
#define VGA_RED			      0xF800
#define VGA_GREEN		      0x0400
#define VGA_BLUE		      0x001F
#define VGA_SILVER		    0xC618
#define VGA_GRAY		      0x8410
#define VGA_MAROON		    0x8000
#define VGA_YELLOW		    0xFFE0
#define VGA_OLIVE		      0x8400
#define VGA_LIME		      0x07E0
#define VGA_AQUA		      0x07FF
#define VGA_TEAL		      0x0410
#define VGA_NAVY		      0x0010
#define VGA_FUCHSIA		    0xF81F
#define VGA_PURPLE		    0x8010
#define VGA_TRANSPARENT	  0xFFFFFFFF
//===========================================================================================
#define swap(type, i, j) { type t = i; i = j; j = t; }
//===========================================================================================
//===========================================================================================
// Структура описания шрифта
//===========================================================================================
typedef struct _current_font
{
  const unsigned char* font;      // Указатель на шрифт
  unsigned int x_size;    // Размер по горизонтали
  unsigned int y_size;    // Размер по вертикали
  unsigned char offset;    // Межбуквенное расстояние
  unsigned char numchars;  // Количество символов
} _current_font;

extern _current_font cfont;

//===========================================================================================
// Структура описания параметров
//===========================================================================================
typedef struct _lcd_param
{
  unsigned char model;     		// Модель дисплея
  unsigned char orient;    		// Ориентация дисплея (Портрет или Альбом)
  unsigned int	disp_x_size;  // Размер экрана по горизонтали
  unsigned int	disp_y_size;  // Размер экрана по вертикали
  unsigned char _transparent; // Вкл/Выкл прозрачность
  unsigned char fnt_color_h;  // Цвет шрифта старший байт
  unsigned char fnt_color_l;  // Цвет шрифтасрфк младший байт
  unsigned char bg_color_h;   // Цвет фона старший байт
  unsigned char bg_color_l;   // Цвет фона младший байт
  unsigned char transfer_mode;// Метод заполнения области экрана
} _lcd_param;

extern _lcd_param lcd_param;

extern void init_BUS(unsigned char display_model, unsigned char orientation);
extern void LCD_Write_COM(unsigned char VL);
extern void LCD_Write_DATA(unsigned char VH, unsigned char VL);
extern void LCD_Write_COM_DATA(unsigned char commmand, int data);
extern void LCD_ClrScreen(void);
extern void LCD_SetFontColor_RGB(unsigned char r, unsigned char g, unsigned char b);
extern void LCD_SetFontColor_Word(unsigned int color);
extern void LCD_SetBackColor_RGB(unsigned char r, unsigned char g, unsigned char b);
extern void LCD_SetBackColor_Word(unsigned int color);
extern void LCD_FillScreen_RGB(unsigned char r, unsigned char g, unsigned char b);
extern void LCD_FillScreen(unsigned int color);
extern unsigned int LCD_GetRegister(unsigned char index);
extern void LCD_DrawPixel(unsigned int x, unsigned int y, unsigned int color);
extern void LCD_DrawLine(long int x1, long int y1, long int x2, long int y2, unsigned int color, long int increment_x);
extern void LCD_DrawRectangle(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned int color);
extern void LCD_DrawArc(int x, int y, int r, unsigned int b_angle, unsigned int angle, unsigned int color);
extern void LCD_FillArc(int x, int y, int r, unsigned int b_angle, unsigned int angle, unsigned int color);
extern void LCD_FillRectangle(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned int color);
extern void LCD_SetFont(const unsigned char *font);
extern void LCD_DrawChar(unsigned int x, unsigned int y, unsigned char ch);
extern void LCD_DrawText(unsigned int x, unsigned int y, unsigned char* text);
extern void LCD_DrawNumberUInt(unsigned int x, unsigned int y, unsigned int val);
extern void LCD_DrawNumberInt(unsigned int x, unsigned int y, int val);
extern void LCD_DrawNumberChar(unsigned int x, unsigned int y, signed char val);
extern void LCD_DrawNumberUChar(unsigned int x, unsigned int y, unsigned char val);
extern void LCD_DrawLineBresen(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned int color);

#define LCD_Bus_Input()	      { LCD_HI_DDR = 0x00;} // LCD Bus as input
#define LCD_Bus_Output()      { LCD_HI_DDR = 0xFF; LCD_HI_PORT = 0xFF;} // LCD Bus as output