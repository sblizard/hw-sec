#include "../ecc.hh"
#include "../verif.hh"

uint32_t xor_bits(uint32_t data, uint8_t bit_included[][16], int32_t bit_num)
{
    uint32_t parity = 0;
    for (size_t i = 0; i < 16; i++)
    {
        parity ^= bit_included[bit_num][i] * getBit(data, i);
    }

    return parity;
}

// Convenience Array
//
// Value for row x column y is 1 iff
// bit y is included in parity bit x
uint8_t parity_eqs[5][16] = {
    {1, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1},
    {1, 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0},
    {0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1},
    {0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1}};

uint32_t genParityBit(uint32_t data)
{
    uint32_t parity = 0;
    for (int i = 0; i < 16; i++)
    {
        parity ^= getBit(data, i);
    }

    return parity;
}

// Generates the parity bits (P5-P0) for a given input ("data")
uint32_t genParity(uint32_t data)
{
    uint32_t parity = 0;

    // Exercise 5-2, Generate the parity bits for the data
    uint32_t p0 = xor_bits(data, parity_eqs, 0);
    uint32_t p1 = xor_bits(data, parity_eqs, 1);
    uint32_t p2 = xor_bits(data, parity_eqs, 2);
    uint32_t p3 = xor_bits(data, parity_eqs, 3);
    uint32_t p4 = xor_bits(data, parity_eqs, 4);
    uint32_t p5 = p0 ^ p1 ^ p2 ^ p3 ^ p4;

    for (int i = 0; i < 16; i++)
    {
        p5 ^= getBit(data, i);
    }

    return (p0) |
           (p1 << 1) |
           (p2 << 2) |
           (p3 << 3) |
           (p4 << 4) |
           (p5 << 5);
}

// Determine if there are any errors, reporting the error type and syndrome.
struct hamming_result findHammingErrors(uint32_t encoded)
{
    // Break down the ECC protected value into parity and data
    hamming_struct decoded = extractEncoding(encoded);

    // Record the parity in the data, and also re-generate the parity
    // from the data
    uint32_t recordedParity = decoded.parity;
    uint32_t regenParity = genParity(decoded.data);

    // Exercise 5-4, Compute the syndrome
    uint32_t parityDiff = recordedParity ^ regenParity;
    uint32_t syndrome = parityDiff & 0x1F;

    // Exercise 5-4, Compute P5 Error bit
    uint32_t P5_Error_bit = 0;
    for (int i = 0; i < TOTAL_BITS; i++)
    {
        P5_Error_bit ^= getBit(encoded, i);
    }

    // Exercise 5-4, Determine the error type
    _ERROR_TYPE error = NO_ERROR;

    if (syndrome == 0 && P5_Error_bit == 0)
    {
        error = NO_ERROR;
    }
    else if (syndrome == 0 && P5_Error_bit == 1)
    {
        error = PARITY_ERROR;
    }
    else if (syndrome != 0 && P5_Error_bit == 1)
    {
        error = SINGLE_ERROR;
    }
    else // syndrome != 0 && P5_Error_bit == 0
    {
        error = DOUBLE_ERROR;
    }

    return {error, syndrome};
}

// Use findHammingErrors to check if there's an error
// If there's an error, correct it!
uint32_t verifyAndRepair(uint32_t encoded)
{
    // Determine the error type
    struct hamming_result result = findHammingErrors(encoded);

    // Exercise 5-4, If the error type is correctable, correct it here!
    uint32_t out = encoded;
    _ERROR_TYPE error = result.error;

    if (error == PARITY_ERROR)
    {
        out = flipBit(out, TOTAL_BITS - 1);
    }
    else if (error == SINGLE_ERROR)
    {
        out = flipBit(out, result.syndrome - 1);
    }

    return out;
}

/*
 *
 * DO NOT MODIFY BELOW ME
 *
 */

int main(void)
{

    /* -------- Setup Phase -------- */

    // Generate Random Data
    int numTests = 5;
    int pass = 1;
    for (int testIter = 0; testIter < numTests; testIter++)
    {
        printf("=== Test Iteration %d / %d ===\n", testIter + 1, numTests);

        srand(time(NULL));
        uint32_t data = rand() % (1 << NUM_DATA_BITS);
        printf("Random Data Generated: %x\n", data);

        // Generate Data Parity
        uint32_t parity = genParity(data);
        printf("Parity of Generated Data: %x\n", parity);

        // Encode into bitstream
        uint32_t encoded = embedEncoding({data, parity});
        printf("Embedded Encoding: %x\n", encoded);

        // Check result against gold solution
        if (!checkParity(encoded))
        {
            printf("FAIL: Not quite! Your parity value is incorrect.\n");
            return -1;
        }
        else
        {
            printf("PASS: Your parity value is correct.\n");
        }

        /* -------- Verify No Error Case --------- */

        // Verify that no errors occur upon 0 bit flips
        printf("\nVerifying (no flips)...\n");
        if (findHammingErrors(encoded).error != NO_ERROR)
        {
            printf("FAIL: Error detected when there wasn't one!\n");
            pass = 0;
        }
        else if (encoded != verifyAndRepair(encoded))
        {
            printf("FAIL: verifyAndRepair changed a valid encoding!\n");
            pass = 0;
        }
        else
        {
            printf("PASS.\n");
        }

        /* -------- Verify One Error Case --------- */

        printf("\nVerifying (one flip)...\n");

        // Inject errors into encoding
        uint32_t encoded_oneflip = injectRandomFlips(encoded, 1);
        printf("Data after one bit flip: %x\n", encoded_oneflip);

        // Verify 1 bit flip case
        uint32_t repaired = verifyAndRepair(encoded_oneflip);
        printf("Repaired Encoding: %x\n", repaired);

        if (findHammingErrors(encoded_oneflip).error != SINGLE_ERROR)
        {
            printf("FAIL: Single error not detected!\n");
            pass = 0;
        }
        else if (encoded != verifyAndRepair(encoded_oneflip))
        {
            printf("FAIL: verifyAndRepair failed to correct value!\n");
            pass = 0;
        }
        else
        {
            printf("PASS.\n");
        }

        /* -------- Verify Parity Error Case --------- */

        printf("\nVerifying (parity flip)...\n");

        // Inject errors into encoding
        uint32_t encoded_parityflip = flipBit(encoded, TOTAL_BITS - 1);
        printf("Data after parity bit flip: %x\n", encoded_parityflip);

        // Verify 1 bit flip case
        repaired = verifyAndRepair(encoded_parityflip);
        printf("Repaired Encoding: %x\n", repaired);

        if (findHammingErrors(encoded_parityflip).error != PARITY_ERROR)
        {
            printf("FAIL: Parity error not detected!\n");
            pass = 0;
        }
        else if (encoded != verifyAndRepair(encoded_parityflip))
        {
            printf("FAIL: verifyAndRepair failed to correct value!\n");
            pass = 0;
        }
        else
        {
            printf("PASS.\n");
        }

        /* -------- Verify Double Error Case --------- */

        printf("\nVerifying (double flip)...\n");

        // Inject 2 errors into encoding
        uint32_t encoded_twoflip = injectRandomFlips(encoded, 2);
        printf("Two bit flip: %x\n", encoded_twoflip);

        if (findHammingErrors(encoded_twoflip).error != DOUBLE_ERROR)
        {
            printf("FAIL: Parity error not detected!\n");
            pass = 0;
        }
        else
        {
            printf("PASS.\n");
        }

        if (!pass)
        {
            printf("\n*** One or more tests failed. ***\n");
            break;
        }
    }
    /* -------- Finish --------- */

    if (pass)
    {
        printf("\n*** All tests passed, congratulations! ***\n");
    }
}
