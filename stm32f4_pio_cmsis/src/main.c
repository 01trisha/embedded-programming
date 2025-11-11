/* main.c — исправленная версия: транспонируем 8x8 glyph (rows -> columns)
   STM32F411CEU6, I2C1 PB6/PB7 -> SSD1315
   Печатает "ILDAR MY BEST FRIEND"
*/

#include "stm32f4xx.h"
#include <string.h>
#include <stdint.h>

#define I2C_ADDR_SSD1315   (0x3C << 1)  // 7-бит адрес 0x3C, сдвинутый влево (используется как в вашем коде)
#define I2C_TIMEOUT         10000U

/* Простая задержка */
static void delay_ms(uint32_t ms) {
    uint32_t i;
    while(ms--) {
        for (i = 0; i < 16000; i++) { __NOP(); }
    }
}

/* Инициализация I2C1 на PB6=SCL, PB7=SDA */
static void I2C1_Init(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

    GPIOB->MODER &= ~((3U << (6*2)) | (3U << (7*2)));
    GPIOB->MODER |=  ((2U << (6*2)) | (2U << (7*2))); // AF mode
    GPIOB->OTYPER |= (1U << 6) | (1U << 7);           // open-drain
    GPIOB->OSPEEDR |= ((3U << (6*2)) | (3U << (7*2))); // high speed
    GPIOB->PUPDR &= ~((3U << (6*2)) | (3U << (7*2)));
    GPIOB->PUPDR |= ((1U << (6*2)) | (1U << (7*2))); // pull-up
    GPIOB->AFR[0] &= ~((0xFU << (6*4)) | (0xFU << (7*4)));
    GPIOB->AFR[0] |= (4U << (6*4)) | (4U << (7*4));   // AF4 for PB6, PB7

    /* I2C timing: предполагаем APB1 = 42 MHz, стандарт 100 kHz */
    I2C1->CR1 &= ~I2C_CR1_PE;
    I2C1->CR2 = 42;            /* PCLK1 clock in MHz */
    I2C1->CCR = 210;           /* 42MHz/(2*100kHz) ≈ 210 */
    I2C1->TRISE = 43;
    I2C1->CR1 |= I2C_CR1_PE;   /* enable I2C1 */
}

/* Блокирующая запись в I2C */
static int I2C1_Write(uint8_t addr, const uint8_t *data, uint32_t len) {
    uint32_t timeout;

    /* START */
    I2C1->CR1 |= I2C_CR1_START;
    timeout = I2C_TIMEOUT;
    while (!(I2C1->SR1 & I2C_SR1_SB)) {
        if (!--timeout) return -1;
    }

    /* Отправка адреса (write) */
    I2C1->DR = addr & ~0x01;
    timeout = I2C_TIMEOUT;
    while (!(I2C1->SR1 & I2C_SR1_ADDR)) {
        if (!--timeout) return -1;
    }
    (void)I2C1->SR2; /* очистка ADDR */

    /* Отправляем байты */
    for (uint32_t i = 0; i < len; i++) {
        timeout = I2C_TIMEOUT;
        while (!(I2C1->SR1 & I2C_SR1_TXE)) {
            if (!--timeout) return -1;
        }
        I2C1->DR = data[i];
    }

    /* дождёмся BTF */
    timeout = I2C_TIMEOUT;
    while (!(I2C1->SR1 & I2C_SR1_BTF)) {
        if (!--timeout) return -1;
    }

    /* STOP */
    I2C1->CR1 |= I2C_CR1_STOP;
    return 0;
}

/* Отправка команды SSD1315 */
static void SSD1315_Command(uint8_t cmd) {
    uint8_t buf[2] = {0x00, cmd}; // control byte: Co=0, D/C#=0
    I2C1_Write(I2C_ADDR_SSD1315, buf, 2);
}

/* Отправка данных SSD1315 чанками (каждый пакет с control=0x40) */
static int SSD1315_Data(const uint8_t *pdata, uint32_t len) {
    const uint32_t CHUNK_MAX = 32;
    uint8_t buf[CHUNK_MAX + 1];
    uint32_t sent = 0;
    while (sent < len) {
        uint32_t chunk = (len - sent > CHUNK_MAX) ? CHUNK_MAX : (len - sent);
        buf[0] = 0x40; /* control: D/C#=1 (data) */
        memcpy(&buf[1], &pdata[sent], chunk);
        if (I2C1_Write(I2C_ADDR_SSD1315, buf, chunk + 1) < 0) return -1;
        sent += chunk;
    }
    return 0;
}

/* Инициализация дисплея */
static void SSD1315_Init(void) {
    delay_ms(100);
    SSD1315_Command(0xAE); // дисплей OFF
    SSD1315_Command(0xD5); SSD1315_Command(0x80);
    SSD1315_Command(0xA8); SSD1315_Command(0x3F);
    SSD1315_Command(0xD3); SSD1315_Command(0x00);
    SSD1315_Command(0x40);
    SSD1315_Command(0x8D); SSD1315_Command(0x14);
    SSD1315_Command(0x20); SSD1315_Command(0x00);
    SSD1315_Command(0xA1); SSD1315_Command(0xC8);
    SSD1315_Command(0xDA); SSD1315_Command(0x12);
    SSD1315_Command(0x81); SSD1315_Command(0xCF);
    SSD1315_Command(0xD9); SSD1315_Command(0xF1);
    SSD1315_Command(0xDB); SSD1315_Command(0x40);
    SSD1315_Command(0xA4); SSD1315_Command(0xA6);
    SSD1315_Command(0xAF); // display ON
    delay_ms(10);
}

/* Минималистичный 8x8 шрифт: пробел и A..Z */
static const uint8_t font8x8_basic[128][8] = {
    [ ' ' ] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    [ 'A' ] = {0x18,0x24,0x24,0x42,0x7E,0x42,0x42,0x00},
    [ 'B' ] = {0x7C,0x42,0x42,0x7C,0x42,0x42,0x7C,0x00},
    [ 'C' ] = {0x3C,0x42,0x80,0x80,0x80,0x42,0x3C,0x00},
    [ 'D' ] = {0x78,0x44,0x42,0x42,0x42,0x44,0x78,0x00},
    [ 'E' ] = {0x7E,0x40,0x40,0x7C,0x40,0x40,0x7E,0x00},
    [ 'F' ] = {0x7E,0x40,0x40,0x7C,0x40,0x40,0x40,0x00},
    [ 'G' ] = {0x3C,0x42,0x80,0x9E,0x82,0x42,0x3C,0x00},
    [ 'H' ] = {0x42,0x42,0x42,0x7E,0x42,0x42,0x42,0x00},
    [ 'I' ] = {0x3E,0x08,0x08,0x08,0x08,0x08,0x3E,0x00},
    [ 'J' ] = {0x1E,0x04,0x04,0x04,0x04,0x44,0x38,0x00},
    [ 'K' ] = {0x42,0x44,0x48,0x70,0x48,0x44,0x42,0x00},
    [ 'L' ] = {0x40,0x40,0x40,0x40,0x40,0x40,0x7E,0x00},
    [ 'M' ] = {0x42,0x66,0x5A,0x5A,0x42,0x42,0x42,0x00},
    [ 'N' ] = {0x42,0x62,0x52,0x4A,0x46,0x42,0x42,0x00},
    [ 'O' ] = {0x3C,0x42,0x81,0x81,0x81,0x42,0x3C,0x00},
    [ 'P' ] = {0x7C,0x42,0x42,0x7C,0x40,0x40,0x40,0x00},
    [ 'Q' ] = {0x3C,0x42,0x81,0x81,0x8D,0x42,0x3D,0x00},
    [ 'R' ] = {0x7C,0x42,0x42,0x7C,0x48,0x44,0x42,0x00},
    [ 'S' ] = {0x3C,0x42,0x40,0x3C,0x02,0x42,0x3C,0x00},
    [ 'T' ] = {0x7F,0x49,0x08,0x08,0x08,0x08,0x1C,0x00},
    [ 'U' ] = {0x42,0x42,0x42,0x42,0x42,0x42,0x3C,0x00},
    [ 'V' ] = {0x42,0x42,0x42,0x24,0x24,0x18,0x18,0x00},
    [ 'W' ] = {0x42,0x42,0x42,0x5A,0x5A,0x66,0x42,0x00},
    [ 'X' ] = {0x42,0x24,0x18,0x18,0x18,0x24,0x42,0x00},
    [ 'Y' ] = {0x41,0x22,0x14,0x08,0x08,0x08,0x1C,0x00},
    [ 'Z' ] = {0x7E,0x02,0x04,0x18,0x20,0x40,0x7E,0x00}
};

/* Установка курсора (column 0..127, page 0..7) */
static void SSD1315_SetCursor(uint8_t column, uint8_t page) {
    SSD1315_Command(0xB0 + (page & 0x0F));
    SSD1315_Command(0x00 + (column & 0x0F));
    SSD1315_Command(0x10 + ((column >> 4) & 0x0F));
}

/* Транспонируем 8 байт-строк (rows[0]..rows[7], MSB=левая) -> 8 байт-столбцов,
   где cols[j] биты идут сверху вниз (бит0 = верхняя строка). */
static void transpose_8x8_rows_to_cols(const uint8_t rows[8], uint8_t cols[8]) {
    for (int j = 0; j < 8; ++j) {
        uint8_t col = 0;
        for (int i = 0; i < 8; ++i) {
            /* rows[i] bit (7 - j) is pixel at (x=j, y=i). Put it into bit i of col */
            if (rows[i] & (1U << (7 - j))) {
                col |= (1U << i);
            }
        }
        cols[j] = col;
    }
}

/* Печать одного символа: транспонируем и отправляем 8 байт-столбцов */
static void SSD1315_WriteChar(char c) {
    const uint8_t *rowBitmap = font8x8_basic[(uint8_t)c]; /* каждый байт = строка */
    uint8_t colBitmap[8];
    transpose_8x8_rows_to_cols(rowBitmap, colBitmap);
    SSD1315_Data(colBitmap, 8);
}

/* Печать строки подряд (без автоматического переноса) */
static void SSD1315_WriteString(const char *s) {
    while (*s) {
        SSD1315_WriteChar(*s++);
    }
}

/* Очистка дисплея */
static void SSD1315_Clear(void) {
    for (uint8_t page = 0; page < 8; page++) {
        SSD1315_SetCursor(0, page);
        for (uint8_t col = 0; col < 128; col += 16) {
            uint8_t zeros[16];
            memset(zeros, 0x00, sizeof(zeros));
            SSD1315_Data(zeros, sizeof(zeros));
        }
    }
}

int main(void) {
    SystemInit();

    I2C1_Init();
    SSD1315_Init();

    SSD1315_Clear();

    /* Поставим курсор в начало (column=0) на страницу 2 для вертикального смещения */

    /* Вывести строку */
    SSD1315_SetCursor(0, 2);
   SSD1315_WriteString("IRISHKA"); 
   SSD1315_SetCursor(0, 4);
   SSD1315_WriteString("LEGA");

    while (1) {
        __WFI();
    }
}
