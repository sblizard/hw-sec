#include "../ecc.hh"
#include "../verif.hh"

// Convenience Array
//
// Value for row x column y is 1 iff
// bit y is included in parity bit x
uint8_t parity_eqs[5][16] = {                 \
    {1,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1},        \
    {1,0,1,1,0,1,1,0,0,1,1,0,1,1,0,0},        \
    {0,1,1,1,0,0,0,1,1,1,1,0,0,0,1,1},        \
    {0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0},        \
    {0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1}
};

// Generates the parity bits (P5-P0) for a given input ("data")
uint32_t genParity(uint32_t data) {
    uint32_t parity = 0;

    // TODO: Exercise 5-2, Generate the parity bits for the data
    int d0 = data & 1;
    int d1 = data >> 1 & 1;
    int d2 = data >> 2 & 1;
    int d3 = data >> 3 & 1;
    int d4 = data >> 4 & 1;
    int d5 = data >> 5 & 1;
    int d6 = data >> 6 & 1;
    int d7 = data >> 7 & 1;
    int d8 = data >> 8 & 1;
    int d9 = data >> 9 & 1;
    int d10 = data >> 10 & 1;
    int d11 = data >> 11 & 1;
    int d12 = data >> 12 & 1;
    int d13 = data >> 13 & 1;
    int d14 = data >> 14 & 1;
    int d15 = data >> 15 & 1;
    int p0 = d15 ^ d13 ^ d11 ^ d10 ^ d8 ^ d6 ^ d4 ^ d3 ^ d1 ^ d0;
    int p1 = d13 ^ d12 & d10 ^ d9 ^ d6 ^ d5 ^ d3 ^ d2 ^ d0;
    int p2 = d15 ^ d14 ^ d10 ^ d9 ^ d8 ^ d7 ^ d3 ^ d2 ^ d1;
    int p3 = d10 ^ d9 ^ d8 ^ d7 ^ d6 ^ d5 ^ d4;
    int p4 = d15 ^ d14 ^ d13 ^ d12 ^ d11;
    int p5 = d0 ^ d1 ^ d2 ^ d3 ^ d4 ^ d5 ^ d6 ^ d7 ^ d8 ^ d9 ^ d10 ^ d11 ^ d12 ^ d13 ^ d14 ^ d15 ^ p0 ^ p1 ^ p2 ^ p3 ^ p4;
    //printf("%d %d %d %d %d %d", p0, p1, p2, p3, p4, p5);
    //int p0 = data >> 15 & 1 ^ data >> 13 & 1 ^ (data >> 11) & 1 ^ (data >> 10) & 1 ^ (data >> 8) & 1 ^ (data >> 6) & 1 ^ (data >> 4) & 1 ^ (data >> 3) & 1 ^ (data >> 1) & 1 ^ data & 1;
    //int p1 = data >> 13 & 1 ^ data >> 12 & 1 ^ data >> 10 & 1 ^ data >> 9 & 1 ^ data >> 6 & 1 ^ data >> 5 & 1 ^ data >> 3 & 1 ^ data >> 2 & 1 ^ data & 1;
    //int p2 = data >> 15 & 1 >> data >> 14 & 1 ^ data >> 10 & 1 ^ data >> 9 & 1 ^ data >> 8 & 1 ^ data >> 7 & 1 ^ data >> 3 & 1 ^ data >> 2 & 1 ^ data >> 1 & 1;
    //int p3 = data >> 10 & 1 ^ data >> 9 & 1 ^ data >> 8 & 1 ^ data >> 7 & 1 ^ data >> 6 & 1 ^ data >> 5 & 1 ^ data >> 4 & 1;
    //int p4 = data >> 15 & 1 ^ data >> 14 & 1 ^ data >> 13 & 1 ^ data >> 12 & 1 ^ data >> 11 & 1;
    //int p5 = data >> 15 & 1 ^ data >> 14 & 1 ^ data >> 13 & 1 ^ data >> 12 & 1 ^ data >> 11 & 1 ^ data >> 10 & 1 ^ data >> 9 & 1 ^ data >> 8 & 1 ^ data >> 7 & 1 ^ data >> 6 & 1 ^ data >> 5 & 1 ^ data >> 4 & 1 ^ data >> 3 & 1 ^ data >> 2 & 1 ^ data >> 1 & 1 ^ data & 1 ^ p0 ^ p1 ^ p2 ^ p3 ^ p4;
    parity = p0 + p1*2 + p2*4 + p3*8 + p4*16 + p5*32;
    //printf("%d", p0 + p1*2);
    //printf("%d %d", p2*4 + p3*8, p4*16 + p5*32);
    return parity;
}

// Determine if there are any errors, reporting the error type and syndrome.
struct hamming_result findHammingErrors(uint32_t encoded) {

    // Break down the ECC protected value into parity and data 
    hamming_struct decoded = extractEncoding(encoded);

    // Record the parity in the data, and also re-generate the parity
    // from the data
    uint32_t recordedParity = decoded.parity;
    uint32_t regenParity = genParity(decoded.data);

    // TODO: Exercise 5-4, Compute the syndrome
    uint32_t syndrome = recordedParity ^ regenParity % 32;

    // TODO: Exercise 5-4, Compute P5 Error bit
    uint32_t P5_Error_bit = decoded.data >> 15 & 1 ^ decoded.data >> 14 & 1 ^ decoded.data >> 13 & 1 ^ decoded.data >> 12 & 1 ^ decoded.data >> 11 & 1 ^ decoded.data >> 10 & 1 ^ decoded.data >> 9 & 1 ^ decoded.data >> 8 & 1 ^ decoded.data >> 7 & 1 ^ decoded.data >> 6 & 1 ^ decoded.data >> 5 & 1 ^ decoded.data >> 4 & 1 ^ decoded.data >> 3 & 1 ^ decoded.data >> 2 & 1 ^ decoded.data >> 1 & 1 ^ decoded.data & 1 ^ decoded.parity >> 5 & 1 ^ decoded.parity >> 4 & 1 ^ decoded.parity >> 3 & 1 ^ decoded.parity >> 2 & 1 ^ decoded.parity >> 1 & 1 ^ decoded.parity & 1;

    // TODO: Exercise 5-4, Determine the error type
    _ERROR_TYPE error = NO_ERROR;
    if(syndrome == 0 && P5_Error_bit == 0) {
        error = NO_ERROR;
    }
    else if(syndrome == 0) {
        error = P5_ERROR;
    }
    else if(P5_Error_bit == 1) {
        error = SINGLE_ERROR;
    }
    else {
        error = DOUBLE_ERROR;
    }
    
    return {error, syndrome};
}

// Use findHammingErrors to check if there's an error
// If there's an error, correct it!
uint32_t verifyAndRepair(uint32_t encoded) {

    // Determine the error type
    struct hamming_result result = findHammingErrors(encoded);

    // TODO: Exercise 5-4, If the error type is correctable, correct it here!
    uint32_t out = encoded;
    if(encoded.error == SINGLE_ERROR) {
        out = out ^ (1 << (syndrome - 1));
    }
    else if(encoded.error == P5_ERROR) {
        out = out ^ (1 << 21);
    }
    
    return out;
}

/*
 *
 * DO NOT MODIFY BELOW ME
 *
 */

int main(void) {

    /* -------- Setup Phase -------- */

    // Generate Random Data
    int numTests = 5;
    int pass = 1;
    for (int testIter = 0; testIter < numTests; testIter++) {
        printf("=== Test Iteration %d / %d ===\n", testIter+1, numTests);

        srand (time(NULL));
        uint32_t data = rand() % (1 << NUM_DATA_BITS);
        printf("Random Data Generated: %x\n", data);

        // Generate Data Parity
        uint32_t parity = genParity(data);
        printf("Parity of Generated Data: %x\n", parity);

        // Encode into bitstream
        uint32_t encoded = embedEncoding({data, parity});
        printf("Embedded Encoding: %x\n", encoded);

        // Check result against gold solution
        if(!checkParity(encoded)) {
            printf("FAIL: Not quite! Your parity value is incorrect.\n");
            return -1;
        } else {
            printf("PASS: Your parity value is correct.\n");
        }

        /* -------- Verify No Error Case --------- */


        // Verify that no errors occur upon 0 bit flips
        printf("\nVerifying (no flips)...\n");
        if (findHammingErrors(encoded).error != NO_ERROR) {
            printf("FAIL: Error detected when there wasn't one!\n");
            pass = 0;
        } else if (encoded != verifyAndRepair(encoded)) {
            printf("FAIL: verifyAndRepair changed a valid encoding!\n");
            pass = 0;
        } else {
            printf("PASS.\n");
        }
        
        /* -------- Verify One Error Case --------- */

        printf("\nVerifying (one flip)...\n");

        // Inject errors into encoding    
        uint32_t encoded_oneflip = injectRandomFlips(encoded,1);
        printf("Data after one bit flip: %x\n", encoded_oneflip);

        // Verify 1 bit flip case    
        uint32_t repaired = verifyAndRepair(encoded_oneflip);
        printf("Repaired Encoding: %x\n", repaired);

        if (findHammingErrors(encoded_oneflip).error != SINGLE_ERROR) {
            printf("FAIL: Single error not detected!\n");
            pass = 0;
        } else if (encoded != verifyAndRepair(encoded_oneflip)) {
            printf("FAIL: verifyAndRepair failed to correct value!\n");
            pass = 0;
        } else {
            printf("PASS.\n");
        }
                
        /* -------- Verify Parity Error Case --------- */
        
        printf("\nVerifying (parity flip)...\n");
        
        // Inject errors into encoding    
        uint32_t encoded_parityflip = flipBit(encoded,TOTAL_BITS-1);
        printf("Data after parity bit flip: %x\n", encoded_parityflip);

        // Verify 1 bit flip case    
        repaired = verifyAndRepair(encoded_parityflip);
        printf("Repaired Encoding: %x\n", repaired);
        
        if (findHammingErrors(encoded_parityflip).error != PARITY_ERROR) {
            printf("FAIL: Parity error not detected!\n");
            pass = 0;
        } else if (encoded != verifyAndRepair(encoded_parityflip)) {
            printf("FAIL: verifyAndRepair failed to correct value!\n");
            pass = 0;
        } else {
            printf("PASS.\n");
        }
        
        /* -------- Verify Double Error Case --------- */
        
        printf("\nVerifying (double flip)...\n");
        
        // Inject 2 errors into encoding
        uint32_t encoded_twoflip = injectRandomFlips(encoded,2);
        printf("Two bit flip: %x\n", encoded_twoflip);

        if (findHammingErrors(encoded_twoflip).error != DOUBLE_ERROR) {
            printf("FAIL: Parity error not detected!\n");
            pass = 0;
        } else {
            printf("PASS.\n");
        }

        if (!pass) {
            printf("\n*** One or more tests failed. ***\n");
            break;
        }
    } 
    /* -------- Finish --------- */

    if(pass) {
        printf("\n*** All tests passed, congratulations! ***\n");
    }

}
