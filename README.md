# syslog-linked-list

Linux syslog kayitlarini okuyup cift yonlu bagli liste (doubly linked list) veri yapisinda saklayan ve yoneten bir C programi.

---

## Proje Hakkinda

Bu program, `/var/log/syslog` gibi bir Linux sistem gunlugu dosyasini satir satir okur. Her satiri ayristirarak (parse) tarih, servis ve mesaj bilgilerini cikarir, ardindan bu kayitlari bellekte cift yonlu bagli liste olarak tutar.

Temel amac; syslog mekanizmasini ve bagli liste veri yapisini birlikte anlamaktir.

---

## Neden Cift Yonlu Bagli Liste?

Syslog dosyalari kronolojik sirali akar — yani yeni kayitlar hep sona eklenir. Cift yonlu bagli liste bu yapiya dogrudan uyum saglar:

| Islem | Karmasiklik | Aciklama |
|---|---|---|
| Sona ekleme | O(1) | `kuyruk` isaretcisi sayesinde |
| Ileri gezinme | O(n) | en eskiden en yeniye |
| Geriye gezinme | O(n) | en yeniden en eskiye (`onceki` isaretcisi) |
| Bellek yonetimi | Dinamik | kayit sayisi onceden bilinmez |

Tek yonlu liste yetmezdi cunku "son N kaydi goster" gibi geriye dogru sorgular yapilamazdi.

---

## Veri Yapisi

```
Liste
 ├── bas     ──►  [Dugum #1] ◄──► [Dugum #2] ◄──► ... ◄──► [Dugum #N]
 ├── kuyruk  ──────────────────────────────────────────────────────►|
 └── boyut   = N


Her Dugum:
 ┌─────────────────────────────┐
 │ veri.no     : int           │
 │ veri.tarih  : char[32]      │
 │ veri.servis : char[64]      │
 │ veri.mesaj  : char[384]     │
 ├─────────────────────────────┤
 │ *onceki  →  bir onceki dugum│
 │ *sonraki →  bir sonraki dugum│
 └─────────────────────────────┘
```

---

## Dosyalar

```
syslog-linked-list/
├── syslog_bagli_liste.c   # Ana kaynak kod
├── sample_syslog.log      # Test icin ornek log dosyasi
└── README.md
```

---

## Derleme ve Calistirma

```bash
# Derle
gcc -o syslog syslog_bagli_liste.c

# Ornek log dosyasiyla calistir
./syslog sample_syslog.log

# Gercek sistem loguyla calistir (root gerekebilir)
./syslog /var/log/syslog
```

---

## Cikti Ornegi

```
Yuklenen kayit sayisi: 35
Ilk kayit (bas)    : Mar 13 08:01:12
Son kayit (kuyruk) : Mar 13 09:30:01

=== TUM KAYITLAR (Toplam: 35) ===
[  1] Mar 13 08:01:12    | kernel    | Linux version 5.15.0-91-generic
[  2] Mar 13 08:01:15    | systemd   | Started System Logging Service.
...

=== ARAMA: "Failed" ===
[ 13] Mar 13 08:22:13 | sshd | Failed password for invalid user hacker...
-> 3 eslesme bulundu.

=== SON 3 KAYIT (geriye gezinme) ===
[ 35] Mar 13 09:30:01 | CRON | (root) CMD (run-parts /etc/cron.hourly)
[ 34] Mar 13 09:25:01 | kernel | TCP: Possible SYN flooding on port 80
[ 33] Mar 13 09:20:33 | sshd | Disconnecting invalid user test...
```

---

## Fonksiyonlar

| Fonksiyon | Aciklama |
|---|---|
| `liste_olustur()` | Bos bir cift yonlu liste olusturur |
| `sona_ekle()` | Yeni kaydi kuyruğa O(1) ile ekler |
| `satiri_parse_et()` | Ham syslog satirini alanlara ayirir |
| `liste_yazdir()` | Tum listeyi ileri yonde yazdirir |
| `kelime_ara()` | Mesaj veya servis alaninda arama yapar |
| `son_kayitlari_yazdir()` | Kuyruktan geriye dogru N kayit gosterir |
| `liste_temizle()` | Tum dugümleri bellekten serbest birakir |

---

## Gereksinimler

- GCC veya herhangi bir C99 uyumlu derleyici
- Linux / macOS / Windows (MinGW)
