# Nero SA-MP: Dialog Layout & Coordination Guide

Dokumen ini menjelaskan bagaimana nilai-nilai visual (padding, size, koordinat) diatur dalam sistem dialog untuk meniru gaya **SAMP PC (1:1)**.

---

## 1. Lokasi File Utama
Hampir seluruh logika tata letak dialog berada di:
- **`jni/jni/gui/samp_widgets/dialogs/dialog.cpp`**: Otak utama (`performLayout`) yang menghitung lebar/tinggi dialog dan posisi komponen besar.
- **`jni/jni/gui/samp_widgets/dialogs/content.cpp`**: Mengatur posisi widget di dalam area konten (list, input, msgbox).
- **`jni/jni/gui/uisettings.cpp`**: Menyimpan nilai default global (font size, warna, tinggi baris list).

---

## 2. Struktur Koordinat Dialog (CDialog)
Dialog dibagi menjadi 3 zona utama secara vertikal:

### A. Title Area (Header)
- **Ketinggian (Height):** Diatur oleh `fHeaderHeight` (Default: `20.0f`).
- **Posisi:** Selalu di `(0, 0)` relatif terhadap dialog.
- **Padding Teks:** Teks judul digeser `10.0f` dari kiri dalam `CDialogTitle::performLayout()`.

### B. Content Area (Tengah)
- **Posisi X (`fContentX`):** Default `20.0f`. Ini adalah jarak dari tepi kiri dialog ke kotak konten.
- **Posisi Y (`fContentY`):** `fHeaderHeight + 10.0f`. Memberikan celah 10px di bawah judul.
- **Padding Samping (`fSidePadding`):** Default `40.0f`. 
    - *Rumus:* `Lebar Dialog = Lebar Konten + 40.0f`.
    - Karena `fContentX` adalah 20, maka sisa di kanan otomatis menjadi 20 (Simetris).

### C. Button Area (Bawah)
- **Posisi Y:** `m_fHeight - 35.0f`. Tombol selalu diletakkan 35px dari dasar dialog.
- **Tinggi Area Tombol:** `30.0f`.

---

## 3. Logika Penempatan Tombol (CDialogButton)
Diatur dalam `CDialogButton::performLayout()`:

- **Dua Tombol:**
    - **Pusat Horizontal (`fCenterX`):** `Lebar Dialog / 2`.
    - **Tombol 1 (Kiri):** `fCenterX - 110.0f`.
    - **Tombol 2 (Kanan):** `fCenterX + 10.0f`.
    - *Catatan:* Jarak antar tombol di PC adalah 20px (dari -10 ke +10 terhadap pusat).
- **Satu Tombol:**
    - Diletakkan di `fCenterX - 50.0f` (Center alignment).

---

## 4. Spesifikasi Per-Style Dialog

### MSGBOX
- **Lebar:** Dihitung dari lebar teks terpanjang + `fSidePadding` (40px).
- **Padding:** 20px kiri, 20px kanan.

### INPUT / PASSWORD
- **Margin Kiri Konten:** Diubah menjadi `5.0f` (Lebih mepet ke tepi untuk meniru PC).
- **Lebar Input Box:** `Lebar Konten + 30.0f` (Agar memenuhi area dialog).

### LIST / TABLIST
- **Internal Right Padding:** Di `dialog.cpp`, ada tambahan `+ 20.0f` pada perhitungan `cx`.
    - *Tujuan:* Memberikan ruang kosong di sisi kanan list (tempat scrollbar di PC) agar teks tidak menempel ke garis dialog.
- **Jarak Antar Kolom (TabList):** Diatur di `tablistwidget.cpp` pada bagian `current_x += w + 30.0f`.
- **Tinggi Baris:** Diambil dari `UISettings::dialogListItemHeight()` (Default: `18.0f` hingga `22.0f`).

---

## 5. Ringkasan Mencari Nilai (Cheat Sheet)

| Jika ingin mengubah... | Lokasi File | Nama Variabel / Fungsi |
| :--- | :--- | :--- |
| **Lebar Dialog Minimal** | `dialog.cpp` | `cx = 190.0f` (di `performLayout`) |
| **Jarak Dialog dari Tepi Layar** | `dialog.cpp` | `this->setPosition(...)` di akhir `performLayout` |
| **Padding Judul ke Atas/Bawah** | `dialog.cpp` | `m_pLabel->setPosition` dalam `CDialogTitle` |
| **Jarak Tombol ke Konten** | `dialog.cpp` | `fClientHeight + 85.0f` (Overhead total) |
| **Warna Background Dialog** | `uisettings.cpp` | `dialogBackgroundColor()` |
| **Spasi Kolom Tablist** | `tablistwidget.cpp` | Nilai `30.0f` di fungsi `assemble` |

---

## 6. Tips Mengatur Ulang
Jika Anda ingin dialog terasa lebih lebar secara keseluruhan tanpa mengubah teks, naikkan nilai `fSidePadding` di `dialog.cpp`. Jika ingin teks di dalam list lebih menjorok ke dalam, ubah nilai `UISettings::padding()` yang digunakan di `listwidget.cpp` atau `tablistwidget.cpp`.
