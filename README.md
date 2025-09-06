# Image Resizer

Bu proje, C dili ile geliştirilmiş bir görüntü boyutlandırma uygulamasıdır. İki farklı versiyon bulunmaktadır. Her iki versiyon da aynı komutlarla derlenip çalıştırılabilir. Ayrıca, derlenmiş dosya mevcutsa doğrudan çalıştırılabilir.

---

## Versiyon 1
**Özellikler**
- Kayan noktalı (floating-point) tabanlı bilineer enterpolasyon
- Gri tonlu ve RGB görüntü desteği
- PNG giriş/çıkış (STB kütüphaneleri ile)
- Donanım uyumluluğu için açık bellek yönetimi

**Çalıştırma**
```bash
gcc -o image_resizer main.c image_resize.c -lm
./image_resizer
```
Derlenmiş dosya mevcutsa doğrudan çalıştırabilirsiniz:
```bash
./image_resizer
```

---

## Versiyon 2
**Özellikler**
- Sabit nokta (fixed-point, Q16.16) tabanlı bilineer enterpolasyon
- Daha donanım uyumlu, hızlı ve deterministik hesaplama
- Nearest neighbor alternatifi
- Geliştirilmiş PNG giriş/çıkış ve RGBA → RGB dönüşümü

**Çalıştırma**
```bash
gcc -o image_resizer main.c image_resize.c -lm
./image_resizer
```
Derlenmiş dosya mevcutsa doğrudan çalıştırabilirsiniz:
```bash
./image_resizer
```
