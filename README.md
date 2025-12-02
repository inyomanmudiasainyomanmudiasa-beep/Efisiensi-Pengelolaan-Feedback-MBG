# Efisiensi Pengelolaan Feedback MBG

Proyek ini menargetkan pembuatan program sederhana untuk mengolah umpan balik (feedback) terkait Program Makan Bergizi Gratis (MBG). Tujuannya adalah mengekstrak kata-kata kunci (keyword) dari feedback siswa/sekolah untuk membantu pengelola mengetahui pola makanan yang disukai atau tidak disukai.

## Fitur
- Pipeline teks dasar:
  1. Case folding (huruf kecil)
  2. Cleaning (menghapus karakter non-huruf, menyimpan spasi dan tanda reduplikasi '-')
  3. Tokenizing (memecah kalimat menjadi kata)
  4. Stopword removal (menghapus kata-kata umum Indonesia)
  5. Simple stemming (heuristik: hapus suffiks umum: -kan, -lah, -nya, -ku, -mu, -an, -i)
- Ekstraksi kata kunci: top-N kata paling sering (berbasis frekuensi).
- Output ramah pengguna: daftar "kata (jumlah kali)".

## File dalam repositori
- `mbg_feedback.cpp` — kode sumber C++ (versi sederhana).
- `README.md` — dokumentasi (file ini).

## Cara kompilasi dan jalankan
1. Pastikan kompilator C++ (g++/clang++) tersedia.
2. Kompilasi:
   ```bash
   g++ -std=c++17 -O2 -o mbg_feedback mbg_feedback.cpp
