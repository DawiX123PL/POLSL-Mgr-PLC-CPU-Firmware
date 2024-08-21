#include "plc_crc.hpp"
#include <boost/crc.hpp>

// This function is written by STMicroelectronics
// @file    stm32f4xx_hal_crc.c
// @author  MCD Application Team
static uint32_t CRC_Handle_8(CRC_HandleTypeDef *hcrc, uint8_t pBuffer[], uint32_t BufferLength)
{
    uint32_t i; /* input data buffer index */
    uint16_t data;
    __IO uint16_t *pReg;

    /* Processing time optimization: 4 bytes are entered in a row with a single word write,
     * last bytes must be carefully fed to the CRC calculator to ensure a correct type
     * handling by the peripheral */
    for (i = 0U; i < (BufferLength / 4U); i++)
    {
        hcrc->Instance->DR = ((uint32_t)pBuffer[4U * i] << 24U) |
                             ((uint32_t)pBuffer[(4U * i) + 1U] << 16U) |
                             ((uint32_t)pBuffer[(4U * i) + 2U] << 8U) |
                             (uint32_t)pBuffer[(4U * i) + 3U];
    }
    /* last bytes specific handling */
    if ((BufferLength % 4U) != 0U)
    {
        if ((BufferLength % 4U) == 1U)
        {
            *(__IO uint8_t *)(__IO void *)(&hcrc->Instance->DR) = pBuffer[4U * i]; /* Derogation MisraC2012 R.11.5 */
        }
        if ((BufferLength % 4U) == 2U)
        {
            data = ((uint16_t)(pBuffer[4U * i]) << 8U) | (uint16_t)pBuffer[(4U * i) + 1U];
            pReg = (__IO uint16_t *)(__IO void *)(&hcrc->Instance->DR); /* Derogation MisraC2012 R.11.5 */
            *pReg = data;
        }
        if ((BufferLength % 4U) == 3U)
        {
            data = ((uint16_t)(pBuffer[4U * i]) << 8U) | (uint16_t)pBuffer[(4U * i) + 1U];
            pReg = (__IO uint16_t *)(__IO void *)(&hcrc->Instance->DR); /* Derogation MisraC2012 R.11.5 */
            *pReg = data;

            *(__IO uint8_t *)(__IO void *)(&hcrc->Instance->DR) = pBuffer[(4U * i) + 2U]; /* Derogation MisraC2012 R.11.5 */
        }
    }

    /* Return the CRC computed value */
    return hcrc->Instance->DR;
}

// Calculate CRC using hardware modules 
// # (NOT THREAD SAFE)
// 
// Polynomial       - 0x4C11DB7
// Initial value    - 0xFFFFFFFF
// Final XOR        - 0x0
// Input Reflected  - NOPE
// Output Reflected - NOPE
uint8_t Crc8CalculateHard(uint8_t *buffer, uint32_t size)
{
    __HAL_CRC_DR_RESET(&hcrc);
    uint32_t crc = CRC_Handle_8(&hcrc, buffer, size);
    return crc;
}

// Calculate CRC using hardware modules
// # (NOT THREAD SAFE)
//
// Polynomial       - 0x4C11DB7
// Initial value    - 0xFFFFFFFF
// Final XOR        - 0x0
// Input Reflected  - NOPE
// Output Reflected - NOPE
uint32_t Crc32CalculateHard(uint8_t *buffer, uint32_t size)
{
    __HAL_CRC_DR_RESET(&hcrc);
    uint32_t crc = CRC_Handle_8(&hcrc, buffer, size);
    return crc;
}

// Calculate CRC using software only
// # (Thread safe)
//
// Polynomial       - 0x4C11DB7
// Initial value    - 0xFFFFFFFF
// Final XOR        - 0x0
// Input Reflected  - NOPE
// Output Reflected - NOPE
uint8_t Crc8CalculateSoft(uint8_t *data, uint32_t size)
{
    constexpr uint32_t bytes = 32;
    constexpr uint32_t polynomial = 0x4C11DB7;
    constexpr uint32_t init_val = 0xFFFFFFFF;
    constexpr uint32_t final_xor = 0x0;
    constexpr bool input_reflected  = false;
    constexpr bool output_reflected = false;

    boost::crc_optimal<bytes, polynomial, init_val, final_xor, input_reflected, output_reflected> crc;

    crc.process_bytes(data, size);
    uint32_t checksum = crc.checksum();
    return checksum;
}

// Calculate CRC using software only
// # (Thread safe)
//
// Polynomial       - 0x4C11DB7
// Initial value    - 0xFFFFFFFF
// Final XOR        - 0x0
// Input Reflected  - NOPE
// Output Reflected - NOPE
uint32_t Crc32CalculateSoft(uint8_t *data, uint32_t size)
{
    constexpr uint32_t bytes = 32;
    constexpr uint32_t polynomial = 0x4C11DB7;
    constexpr uint32_t init_val = 0xFFFFFFFF;
    constexpr uint32_t final_xor = 0x0;
    constexpr bool input_reflected  = false;
    constexpr bool output_reflected = false;

    boost::crc_optimal<bytes, polynomial, init_val, final_xor, input_reflected, output_reflected> crc;

    crc.process_bytes(data, size);
    uint32_t checksum = crc.checksum();
    return checksum;
}

// # NOT THREAD SAFE
// verifies CRC of dataframe.
// Function expect last byte to be crc.
// returns:
//  - true if crc is correct
//  - false on crc mismatch
bool Crc8Verify(uint8_t *buffer, uint32_t size)
{
    assert_param(size != 0);
    uint8_t calculated_crc = Crc8CalculateHard(buffer, size - 1);
    uint8_t frame_crc = buffer[size - 1];

    return calculated_crc == frame_crc;
}
