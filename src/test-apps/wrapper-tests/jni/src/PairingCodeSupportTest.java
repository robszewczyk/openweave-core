/*
 *
 *    Copyright (c) 2018 Nest Labs, Inc.
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

import static org.junit.Assert.*;

import org.junit.*;
import com.nestlabs.weave.security.PairingCodeSupport;

public class PairingCodeSupportTest extends SimpleTest {

    private static final String[] VALID_PARING_CODES = {
        "NESTUS",
        "TW1TRE",
        "KS7WD2",
        "G532DM",
        "WFFAYAM6W9S9YWXFB62",
        "KLSHDFP987Y2934P58R7USJHDFKLJSHDKLAJHSDGV",
    };

    @Test
    public void testValidCheck() throws Exception
    {
        int mutationIndex = 0;

        for (final String pairingCode: VALID_PARING_CODES) {
            assertTrue(String.format("Unexpected value (false) returned by PairingCodeSupport.isValidPairingCode() for %s", pairingCode), PairingCodeSupport.isValidPairingCode(pairingCode));

            String mutatedPairingCode = mutatePairingCode(pairingCode, mutationIndex);

            assertFalse(String.format("Unexpected value (true) returned by PairingCodeSupport.isValidPairingCode() for %s", mutatedPairingCode), PairingCodeSupport.isValidPairingCode(mutatedPairingCode));

            mutationIndex += 1;
        }

        for (char ch = 0; ch < 0x7F; ch++) {
            boolean expectedResult = (PairingCodeSupport.PAIRING_CODE_CHARS.indexOf(Character.toUpperCase(ch)) != -1);
            boolean result = PairingCodeSupport.isValidPairingCodeChar(ch);
            assertTrue(String.format("Unexpected value (%s) returned by PairingCodeSupport.isValidPairingCodeChar() for '%c'", result, ch), result == expectedResult);
        }
    }

    @Test
    public void testCheckChar() throws Exception
    {
        for (final String pairingCode: VALID_PARING_CODES) {
            String pairingCodeWithoutCheckChar = pairingCode.substring(0, pairingCode.length() - 1);
            char expectedCheckChar = pairingCode.charAt(pairingCode.length() - 1);

            char checkChar = PairingCodeSupport.computeCheckChar(pairingCodeWithoutCheckChar);
            assertTrue(String.format("Unexpected value returned by PairingCodeSupport.computeCheckChar(): %c", checkChar), checkChar == expectedCheckChar);

            String generatedPairingCode = PairingCodeSupport.addCheckChar(pairingCodeWithoutCheckChar);
            assertTrue(String.format("Unexpected value returned by PairingCodeSupport.addCheckChar(): %s", generatedPairingCode), generatedPairingCode.equals(pairingCode));
        }
    }

    @Test
    public void testIntEncodeDecode() throws Exception
    {
        final int kPairingCodeLength = 5;
        final int kNumPairingCodes = 1 << ((kPairingCodeLength - 1) * PairingCodeSupport.BITS_PER_CHARACTER);

        /// For all possible 5 character pairing codes...
        for (long pairingCodeNum = 0; pairingCodeNum < kNumPairingCodes; pairingCodeNum++) {
            // Generate the current pairing code.
            String pairingCode = PairingCodeSupport.intToPairingCode(pairingCodeNum, kPairingCodeLength);

            checkPairingCodeChars(pairingCode);

            long decodedPairingCodeNum = PairingCodeSupport.pairingCodeToInt(pairingCode);

            if (decodedPairingCodeNum != pairingCodeNum) {
                throw new Exception(String.format("Invalid value returned by PairingCodeSupport.pairingCodeToInt(): %d, expected %d", decodedPairingCodeNum, pairingCodeNum));
            }
        }
    }

    @Test
    public void testNormalization() throws Exception
    {
        String normalizedPairingCode;

        // Test normalization of 'I', 'O', 'Q' and 'Z' to '1', '0', '0' and '2'
        normalizedPairingCode = PairingCodeSupport.normalizePairingCode("HZOWQI");
        assertTrue(String.format("Invalid value returned by PairingCodeSupport.normalizePairingCode(): %s", normalizedPairingCode), normalizedPairingCode.equals("H20W01"));

        // Test removal of simple whitespace (' ', '\t', '\r', '\n') and punctuation ('-', '.').
        normalizedPairingCode = PairingCodeSupport.normalizePairingCode("  H\r\n\nR-D-W6.7\t");
        assertTrue(String.format("Invalid value returned by PairingCodeSupport.normalizePairingCode(): %s", normalizedPairingCode), normalizedPairingCode.equals("HRDW67"));
    }

    @Test
    public void testGeneration() throws Exception
    {
        for (int pairingCodeLen = PairingCodeSupport.MIN_PAIRING_CODE_LENGTH; pairingCodeLen < 20; pairingCodeLen++) {
            for (int i = 0; i < 100; i++) {
                String pairingCode = PairingCodeSupport.generatePairingCode(pairingCodeLen);
                checkPairingCodeChars(pairingCode);
                assertTrue(String.format("Invalid value returned by PairingCodeSupport.generatePairingCode(): %s", pairingCode), PairingCodeSupport.isValidPairingCode(pairingCode));
            }
        }
    }

    private static void checkPairingCodeChars(String pairingCode) throws Exception
    {
        for (final char ch: pairingCode.toCharArray()) {
            assertFalse("Invalid character in pairing code", PairingCodeSupport.PAIRING_CODE_CHARS.indexOf(ch) == -1);
        }
    }

    private static String mutatePairingCode(String pairingCode, int index)
    {
        char[] pairingCodeChars = pairingCode.toCharArray();

        index = index % pairingCodeChars.length;

        int mutateCharVal = PairingCodeSupport.PAIRING_CODE_CHARS.indexOf(pairingCodeChars[index]);

        mutateCharVal ^= 0x1F;

        pairingCodeChars[index] = PairingCodeSupport.PAIRING_CODE_CHARS.charAt(mutateCharVal);

        return pairingCodeChars.toString();
    }
}
