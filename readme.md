# perkenalan
ini adalah repo program kontroller automatic gate noborejo yang ada di workshop noborejo. kontrollernya menggunakan arduino nano dan peripherals yang digunakan diantaranya stepper driver, motor stepper, limit switch, receiver radio, 3 buah power supply, dan 2 step down untuk power supply mcu dan receiver radio.
# inspirasi
program kontroller automatic gate terinspirasi dari program homing dan program pergerakan cnc. sehingga di sini dapat ditemukan homing, dan kontrol pergerakan mengunakan letak koordinat.
# cara menggunakan
### penggunaan sehari-hari
* untuk penggunaan sehari-hari, caranya sangat mudah. tinggal nyalakan power
* kemudian kalian pencet tombol on untuk membuka gerbang dan off untuk menutup gerbang
### melakukan homing
* matikan power, kemudian hidupkan lagi
* tekan tombol on untuk membuka gerbang, sekaligus melakukan homing
* homing sudah berada di posisi. sekarang automatic gate dapat digunakan sebagaimana mestinya

# TODO:
* buat program homing ketika pertama kali booting, seperti yang ada di atc senday (done 16/3/2024)
* kalibrasi ulang di lapangan (done 16/3/2024)

# test fungsi senin 15 april 2024
### homing
posisi default homing adalah terbuka
* homing => berfungsi
* reset (dicabut power lalu nyalakan lagi), kemudian homing => berfungsi
* homing dari tengah jalan => berfungsi
* homing saat sudah terbuka maksimal => berfungsi
### running 
ketika on pintu gerbang terbuka dan off pintu gerbang tertutup
* ketika ditekan tombol on => berfungsi, pintu berhasil terbuka
* ketika ditekan tombol off => berfungsi, pintu berhasil tertutup
### masalah yang diketahui
* terkadang tidak dapat terbuka/tertutup ketika remote dioperasikan
* homing tidak bisa dilakukan 
* menabrak, walaupun masih minim (solved) (update firmware)

# update firmware 3 mei 2024
### masalah
* bouncing pada gerbang keika gerbang terbuka / tertutup yang disebabkan oleh menabrak, menyebabkan gerbang terkadang tidak dapat dibuka tutup lagi
* lampu pada gerbang harusnya terbuka dan ke-terbuka adalah hijau, tertutup dan ke-tertutup adalah merah
* lampu kurang terang, sehingga pada siang hari yang terik tidak bisa terlihat dengan jelas.  
### solusi
* update firmware untuk mengatasi semua itu dengan menaikkan nilai deboune dan menurunkan nilai travel
* sudah disesuaikan dengan firmware
* perlu update jenis lampu atau pada bagian atas lampu diberi sebuah payung tambahan penghadadng silalunya sinar matahari