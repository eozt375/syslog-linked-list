/*
 * Syslog Kayitlarini Cift Yonlu Bagli Liste ile Gosteren Program
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SATIR  512
#define LOG_DOSYA  "C:/Users/Enes/Desktop/projeler/C/odev/syslog/test_syslog.txt"

/* --- VERI YAPILARI --- */

/* Her syslog satirindan elde edilen bilgileri tutar */
typedef struct {
    int  no;
    char tarih[32];
    char servis[64];
    char mesaj[384];
} SyslogKaydi;

/* Cift yonlu bagli listenin tek bir dugumu */
typedef struct Dugum {
    SyslogKaydi  veri;
    struct Dugum *onceki;
    struct Dugum *sonraki;
} Dugum;

/* Listenin tamamini yoneten yapi */
typedef struct {
    Dugum *bas;     /* en eski kayit */
    Dugum *kuyruk;  /* en yeni kayit */
    int    boyut;
} Liste;

/* --- BAGLI LISTE FONKSIYONLARI --- */

/* Bos bir liste olusturur */
Liste* liste_olustur() {
    Liste *l = (Liste*)malloc(sizeof(Liste));
    l->bas    = NULL;
    l->kuyruk = NULL;
    l->boyut  = 0;
    return l;
}

/* Yeni kaydi listenin SONUNA ekler - O(1) */
void sona_ekle(Liste *l, SyslogKaydi kayit) {
    Dugum *yeni = (Dugum*)malloc(sizeof(Dugum));
    yeni->veri    = kayit;
    yeni->onceki  = NULL;
    yeni->sonraki = NULL;
    yeni->veri.no = l->boyut + 1;

    if (l->bas == NULL) {
        /* Liste bos: ilk dugum hem bas hem kuyruk */
        l->bas    = yeni;
        l->kuyruk = yeni;
    } else {
        /* Mevcut kuyrugun sonrasina bagla */
        yeni->onceki       = l->kuyruk;
        l->kuyruk->sonraki = yeni;
        l->kuyruk          = yeni;
    }
    l->boyut++;
}

/* Tum dugumleri bellekten siler */
void liste_temizle(Liste *l) {
    Dugum *simdiki = l->bas;
    while (simdiki != NULL) {
        Dugum *silinecek = simdiki;
        simdiki = simdiki->sonraki;
        free(silinecek);
    }
    free(l);
}

/* --- PARSE FONKSIYONU --- */

/*
 * Ham syslog satirini parcalayip SyslogKaydi yapisini doldurur.
 * Format: "Mar 13 08:01:12 hostname servis[PID]: mesaj"
 */
void satiri_parse_et(const char *satir, SyslogKaydi *kayit) {
    char ay[8], gun[4], saat[12], kaynak[64], servis_pid[64], mesaj[384];

    int n = sscanf(satir, "%7s %3s %11s %63s %63[^:]: %383[^\n]",
                   ay, gun, saat, kaynak, servis_pid, mesaj);

    if (n >= 5) {
        snprintf(kayit->tarih, sizeof(kayit->tarih), "%s %s %s", ay, gun, saat);

        /* PID varsa kes: "sshd[1234]" -> "sshd" */
        char *kose = strchr(servis_pid, '[');
        if (kose) *kose = '\0';
        strncpy(kayit->servis, servis_pid, sizeof(kayit->servis) - 1);

        if (n == 6)
            strncpy(kayit->mesaj, mesaj, sizeof(kayit->mesaj) - 1);
        else
            strncpy(kayit->mesaj, "(mesaj yok)", sizeof(kayit->mesaj) - 1);
    } else {
        strncpy(kayit->tarih,  "?", sizeof(kayit->tarih)  - 1);
        strncpy(kayit->servis, "?", sizeof(kayit->servis) - 1);
        strncpy(kayit->mesaj, satir, sizeof(kayit->mesaj) - 1);
    }
}

/* --- GORUNTULEME FONKSIYONLARI --- */

/* Tum listeyi bas->kuyruk yonunde (ileri) yazdirir */
void liste_yazdir(Liste *l) {
    printf("\n=== TUM KAYITLAR (Toplam: %d) ===\n", l->boyut);
    Dugum *d = l->bas;
    while (d != NULL) {
        printf("[%3d] %-18s | %-20s | %.60s\n",
               d->veri.no, d->veri.tarih,
               d->veri.servis, d->veri.mesaj);
        d = d->sonraki; /* ileri git */
    }
}

/* Belirtilen kelimeyi iceren kayitlari bulur */
void kelime_ara(Liste *l, const char *kelime) {
    printf("\n=== ARAMA: \"%s\" ===\n", kelime);
    int bulunan = 0;
    Dugum *d = l->bas;
    while (d != NULL) {
        if (strstr(d->veri.mesaj, kelime) || strstr(d->veri.servis, kelime)) {
            printf("[%3d] %s | %s | %s\n",
                   d->veri.no, d->veri.tarih,
                   d->veri.servis, d->veri.mesaj);
            bulunan++;
        }
        d = d->sonraki;
    }
    printf("-> %d eslesme bulundu.\n", bulunan);
}

/* Son N kaydi kuyruktan geriye dogru yazdirir */
void son_kayitlari_yazdir(Liste *l, int n) {
    printf("\n=== SON %d KAYIT (geriye gezinme) ===\n", n);
    Dugum *d = l->kuyruk;
    int sayac = 0;
    while (d != NULL && sayac < n) {
        printf("[%3d] %s | %s | %s\n",
               d->veri.no, d->veri.tarih,
               d->veri.servis, d->veri.mesaj);
        d = d->onceki; /* geri git */
        sayac++;
    }
}

/* --- ANA FONKSIYON --- */

int main(int argc, char *argv[]) {
    const char *dosya = (argc > 1) ? argv[1] : LOG_DOSYA;

    /* 1. Listeyi olustur */
    Liste *liste = liste_olustur();

    /* 2. Log dosyasini oku, her satiri parse edip listeye ekle */
    FILE *f = fopen(dosya, "r");
    if (!f) {
        printf("Hata: %s acilamadi!\n", dosya);
        return 1;
    }

    char satir[MAX_SATIR];
    while (fgets(satir, MAX_SATIR, f)) {
        if (strlen(satir) < 10) continue;
        SyslogKaydi kayit;
        memset(&kayit, 0, sizeof(kayit));
        satiri_parse_et(satir, &kayit);
        sona_ekle(liste, kayit);
    }
    fclose(f);

    printf("Yuklenen kayit sayisi: %d\n", liste->boyut);
    printf("Ilk kayit (bas)    : %s\n", liste->bas->veri.tarih);
    printf("Son kayit (kuyruk) : %s\n", liste->kuyruk->veri.tarih);

    /* 3. Tum kayitlari listele (ileri gezinme) */
    liste_yazdir(liste);

    /* 4. Kelime ara */
    kelime_ara(liste, "Failed");
    kelime_ara(liste, "sshd");

    /* 5. Son 3 kaydi goster (geri gezinme) */
    son_kayitlari_yazdir(liste, 3);

    /* 6. Bellegi temizle */
    liste_temizle(liste);
    printf("\nBellek temizlendi. Program bitti.\n");

    return 0;
}