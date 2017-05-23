using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;

namespace Emconit.Library {
    public class TylerData {
        public static byte[] EncryptString(string a) {
            byte[] byteA = ASCIIEncoding.ASCII.GetBytes(a);
            byte[] encodedByteA = new byte[byteA.Length];
            for(int i = 0; i < byteA.Length; i++) {
                encodedByteA[i] = Encrypt(byteA[i]);
            }

            return encodedByteA;
        }

        public static string DecryptBytes(byte[] a) {
            string aA = "";
            for(int i = 0; i < a.Length; i++) {
                aA += (char)Encrypt(a[i]);
            }

            return aA;
        }

        public static byte[] DecryptFile(string path) {
            using (FileStream fs = File.OpenRead(path)) {
                byte[] b = new byte[fs.Length];
                for (int i = 0; i < fs.Length; i++) {
                    byte fileByte = (byte)fs.ReadByte();
                    byte newFileByte = Decrypt(fileByte);
                    b[i] = newFileByte;
                }

                return b;
            }
        }

        public static byte Encrypt(byte a) {
            byte newA = 0;
            for(int i = 0; i <= 8; i++) {
                byte aBit = (byte)(IsBitOn(a, i) ? 1:0);
                byte newABit = (byte)(aBit ^ 1);
                if(IsBitOn(newABit, 0)) {
                    newA |= (byte)(1 << i);
                }
            }

            return newA;
        }

        public static byte Decrypt(byte a) {
            return Encrypt(a);
        }
        
        public static bool IsBitOn(byte a, int bitPos) {
            return ((byte)((a >> bitPos) & 1)) == 1;
        }
    }
}
