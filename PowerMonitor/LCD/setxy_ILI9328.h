LCD_Write_COM_DATA(0x20,x1);
LCD_Write_COM_DATA(0x21,y2); // для портретного режима должно быть y1
LCD_Write_COM_DATA(0x50,x1);
LCD_Write_COM_DATA(0x52,y1);
LCD_Write_COM_DATA(0x51,x2);
LCD_Write_COM_DATA(0x53,y2);
LCD_Write_COM(0x22);