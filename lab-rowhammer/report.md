## 1-2

**In a 64-bit system using 4KB pages, which bits are used to represent the page offset, and which are used to represent the page number?**
are used to represent the page number?
Bits 0-11: page offset
Bits 12:63: page number

**How about for a 64-bit system using 2MB pages? Which bits are used for page number and which are for page offset?**
Bits 0-20: page offset
Bits 21-63: page number

**In a 2GB buffer, how many 2MB hugepages are there?**
1024 hugepages

## 2-1

**Given a victim address 0x96ec3000, what is the value of its Row id? The value of its Column id?**
Row id: 0x4b76
Column id: 0x1000

**For the same address, assume an arbitrary XOR function for computing the Bank id, list all possible attacker addresses whose Row id is one more than 0x752C3000's Row id and all the other ids match, including the Bank id and Column id. Hint: there should be 16 such addresses total.**
0x96ee1000
0x96ee3000
0x96ee5000
0x96ee7000
0x96ee9000
0x96eeb000
0x96eed000
0x96eef000
0x96ef1000
0x96ef3000
0x96ef5000
0x96ef7000
0x96ef9000
0x96efb000
0x96efd000
0x96eff000

## 2-3

**Analyze the statistics produced by your code when running part2, and report a threshold to distinguish the bank conflict.**
Threshold = 270 cycles

## 3-2

**Based on the XOR function you reverse-engineered, determine which of the 16 candidate addresses you derived in Discussion Question 2-1 maps to the same bank.**
0x96ee7000

## 4-2

**Try different data pattern and include the bitflip observation statistics in the table below. Then answer the following questions:**
| Data Pattern (Victim/Aggressor) | 0x00/0xff | 0xff/0x00 | 0x00/0x00 | 0xff/0xff |
|---------------------------------|-----------|-----------|-----------|-----------|
| Number of Flips (100 trials) | 3515521 | 3861509 | 0 | 3 |

**Do your results match your expectations? What is the best pattern to trigger flips effectively?**
These match my expectations. The "striped" arrangments yielded the best results compared to the uniform tests. Taking advantage of the capacitor differences gave us better results. The best pattern was 0xff/0x00 for this specific machine and DRAM specs.

## 5-1

**Given the ECC type descriptions listed above, fill in the following table (assuming a data length of 4). For correction/detection, only answer "Yes" if it can always correct/detect (and "No" if there is ever a case where the scheme can fail to correct/detect). We've filled in the first line for you.**
| Metric | 1-Repetition (No ECC) | 2-Repetition | 3-Repetition | Single Parity Bit | Hamming (7,4) |
|--------------------------------------|-----------------------|--------------|--------------|-------------------|---------------|
| Code Rate (Data Bits / Total Bits) | 1.0 | 0.5| 0.33| 0.8 | 0.57 |
| Max Number of Errors Can Detect | 0 | 4 | 8 | 3 | 3 |
| Max Number of Errors Can Correct | 0 | 4 | 4 | 0 | 1 |

## 5-3

**When a single bit flip is detected, describe how Hamming(22,16) can correct this error.**
If syndrom = 0, overall parity = 1:
    Flip overall parity bit
If syndrom = n > 0, overall parity = 1:
    Flip bit n

## 5-5

**Can the Hamming(22,16) code we implemented always protect us from rowhammer attacks? If not, describe how a clever attacker could work around this scheme.**
Blacksmith has shown us that rowhammer can be used to flip multiple bits simultaneously. If we flip multiple bits in the same hamming codeword, an attacker can either flip the bits to an unrepareable state, or ideally, transform one codeword into another valid codeword through the flipping of multiple specific bits.  

