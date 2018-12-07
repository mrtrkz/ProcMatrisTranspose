#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>//proc dosyasını okuma yazma metodu için gerekli modül
#include <asm/uaccess.h>//kullanıcıdan gelen verileri okumak için gerekli modül

#define PROC_TEST_DIR "odev3"//proc dizini ismi
#define PROC_TEST_CONFIG_FILE "matris"//proc dosyası ismi

struct proc_dir_entry *proc_test_dir, *proc_test_config;

#define CONFIG_BUFF_MAX_SIZE 100
//kullanıcıdan gelecek verileri tutmak için kullanılacak dizi
#define MAX_CONFIG_ENTRIES 4
static char proc_test_config_buff[CONFIG_BUFF_MAX_SIZE] = "0 0";
//işlem sonuçlarını ekrana yazmak için kullanılacak string
static char proc_test_config_buff_format[] = " ";

//satır sütün bilgisini ve kaç kere yazma işlemi yapıldı tutacak değişkenler
static int satir=0, sutun=0, sayac=0;

//kullanıcıdan gelen veriyi geçici tutan string
static char proc_test_config_buff_tmp[CONFIG_BUFF_MAX_SIZE] = "0 0";

//kullanıcıdan gelen satır bilgilerinin tutulduğu dizi
#define MAX_CONFIG_ENTRIES2 16
static int proc_test_config_buff2[MAX_CONFIG_ENTRIES2] = {0,0};


static int proc_test_config_open(struct inode *, struct file *);
static ssize_t proc_test_config_write(struct file *,
        const char __user *, size_t , loff_t *);
//dosya işlemleri için metodlar
static struct file_operations proc_test_config_file_ops = {
        .owner = THIS_MODULE,
        .open = proc_test_config_open,
        .read = seq_read,
        .write = proc_test_config_write,
        .llseek = seq_lseek,
        .release = seq_release,
};
//seq_file işlemleri için metodlar
static void *proc_test_config_start(struct seq_file *, loff_t *);
static void *proc_test_config_next(struct seq_file *, void *, loff_t *);
static void proc_test_config_stop(struct seq_file *, void *);
static int proc_test_config_read(struct seq_file *, void *);

static struct seq_operations proc_test_config_seq_op = {
        .start =        proc_test_config_start,
        .next =         proc_test_config_next,
        .stop =         proc_test_config_stop,
        .show =         proc_test_config_read
};


static int proc_test_config_open(struct inode *inode, struct file *file)
{
        return seq_open(file, &proc_test_config_seq_op);
}

static void *proc_test_config_start(struct seq_file *m, loff_t * pos)
{
        static int cur_array_num=1, entry_num = 0;
	//yeni okuma çevrimi başladığında çalışır
        //cat ile proc dosyası okunduğunda kullanıcının göreceği mesaj oluşturuluyor
        if (*pos == 0) {
                m->private = (void *)&cur_array_num;
                seq_printf(m, "\n\nEn Son Kullanıcıdan Alınan Bilgi:\n");
                seq_printf(m, "----------------------------\n");
                return &entry_num;
        } else if (*pos == 1) {
                cur_array_num++;
                entry_num=0;
                m->private = (void *)&cur_array_num;
                seq_printf(m, "\n\nKullanıcıdan Gelen Bilgiler:\n");
                seq_printf(m, "----------------------------\n");
                return &entry_num;
        } else {
                *pos = 0;
                cur_array_num=1;
                return NULL;
        }
}

//sonraki okuma çevrimde ekrana yazılacak bilgiler
static void *proc_test_config_next(struct seq_file *m, void *entry_num,
        loff_t * pos)
{
    int cur_array_num=*((int *)m->private);

    if (cur_array_num == 1)
    {
        *pos = 1;
        return NULL;
    } else if (cur_array_num == 2) {
        if (*((int *)entry_num) < MAX_CONFIG_ENTRIES2)
        {
            (*(int *)entry_num)++;
            return entry_num;
        } else
            return NULL;
    }

    return NULL;
}

//okuma çevrimi tamamlandığında yapılanlar
static void proc_test_config_stop(struct seq_file *m, void *entry_num)
{
    if (*((int *)m->private) == 1)
    {
        return;
    } else if (*((int *)m->private) == 2) {
        seq_printf(m, "\n\n ");
        return;
    }
}

//proc dosyasına yazma işlemi
static int proc_test_config_read(struct seq_file *m, void *entry_num)
{
    int cur_array_num=*((int *)m->private);
	//sprintf için geçici değişken
	char gecici[] = " ";
    if (cur_array_num == 1)
    {
	//satır sayısı kadar kullanıcıdan bilgi alındıysa. satır indeksi 0' dan başladığı için sayac-1 yapıldı
	if((sayac-1)==satir){
		//burada tek boyutlu diziyi iki boyutlu diziye aktarıldı
		int k,l,tmp, matrisG[satir][sutun];
		for(k=0; k<satir; k++)
    		{
			for(l=0; l<sutun; l++){
				matrisG[k][l]=proc_test_config_buff2[l+(k*(sutun))];
			}
		}
		// Bu bölümde matrisin transpozu alınıyor
		for(k=0; k<satir; k++)
			for(l=k+1; l<sutun; l++){
				tmp=matrisG[k][l];
				matrisG[k][l]=matrisG[l][k];
				matrisG[l][k]=tmp;
			}
		// Bu bölümde matris biçiminde ekrana yazacak string hazırlanıyor.
		for(k=0; k<satir; k++){
			for(l=0; l<sutun; l++)
			{
				//chardan stringe dönüştürme
				sprintf(gecici,"%d",matrisG[k][l]);
				strcat(proc_test_config_buff_format,gecici);
				strcat(proc_test_config_buff_format,"\t");
			}
			strcat(proc_test_config_buff_format,"\n");
		}
		seq_printf(m, "\nMatrisin Transpozu\n%s \n\n", proc_test_config_buff_format);
		sayac=0;
	}else
		//eğer girilen satır bilgisi kadar satır kullanıcıdan alınmadıysa gösterilecek mesaj 
		seq_printf(m, "\n%s \n\n olması gereken satır sayısı %d eksik bilgi var. girilen satir %d \n\n", proc_test_config_buff, satir, (sayac-1));
    } else if (cur_array_num == 2) {
	//kullanıcıdan gelen satır bilgilerini diziye yaz
        if (*((int*)entry_num) < MAX_CONFIG_ENTRIES2)
            seq_printf(m, "%d ", proc_test_config_buff2[*((int *)entry_num)]);
    }

    return 0;
}

//kullanıcının girdiği değerlerin rakam olduğu kontrolü
static int isdigit(char c) {
        return ((c >= '0') && (c <= '9'));
}

static ssize_t proc_test_config_write(struct file *file,
        const char __user *buffer, size_t size, loff_t *ppos)
{
    char *buff, *buff1, **end_ptr = NULL;
    unsigned int i;

sayac++;//kaç kez okuma yaptık;
if (sayac==1) sprintf(proc_test_config_buff_format," ");
//her satır okuma işlemi tamamlandığında  matris işlem sonucunu gösteren stringin içi boşaltılır
    if (size > CONFIG_BUFF_MAX_SIZE)
        return 0;
	//kullanıcının girdiği bilgileri al
    raw_copy_from_user(proc_test_config_buff_tmp, buffer, size);
    proc_test_config_buff_tmp[size] = '\0';

    buff = proc_test_config_buff_tmp;
	//kullanıcıdan gelen bilgiler karakter karakter okunur
    for (i=0; i< MAX_CONFIG_ENTRIES; i++)
    {
        buff1 = buff;
        while (isdigit(*buff))
                buff++;
        if (*buff != '\n') {
                if (*buff == ' ')
                        buff++;
                else
                        return 0;
        }
	//sayacın bir olması demek kullanıcı satır ve sütun bilgisini giriyor
        if (sayac==1){
		switch (i) {
		    case 0:
			satir=simple_strtoul(buff1, end_ptr, 10);
		        break;
		    case 1:
			sutun=simple_strtoul(buff1, end_ptr, 10);
		        break;
        	}	
	}else{
		//gelen veri diziye alınıyor
		proc_test_config_buff2[i+(satir*(sayac-2))]=simple_strtoul(buff1, end_ptr, 10);
	}
    }
    *buff = '\0';
	//kernel loguna satır sütün ve sayac bilgileri yazılıyor
	printk(KERN_INFO "Satır:%d Sutun:%d Sayac:%d",satir,sutun,sayac);

    //geçiciden asıl stringe bilgiler kopyalanıyor
    strcpy(proc_test_config_buff, proc_test_config_buff_tmp);

    return size;
}

int proc_test_init(void)
{

        /* /proc dizini altına yeni klasör oluştur */
        proc_test_dir = proc_mkdir(PROC_TEST_DIR, NULL);
        if (!proc_test_dir)
        {
            printk("PROC_DIR Oluşturulamadı...\r\n");
            return -1;
        }

        // matris dosyası oluşturuluyor ve izinler "rw- r-- r--" mode
        proc_test_config = proc_create(PROC_TEST_CONFIG_FILE, 0644,
                                proc_test_dir, &proc_test_config_file_ops);

        if (proc_test_dir == NULL) {
                printk("Dosya oluşturulamadı... /proc/%s/%s\n", PROC_TEST_DIR,
                       PROC_TEST_CONFIG_FILE);
                remove_proc_entry(PROC_TEST_DIR, NULL);
                return -1;
        }

        printk("/proc/%s/%s oluşturuldu\n",
               PROC_TEST_DIR, PROC_TEST_CONFIG_FILE);

        return 0;
}

//modül rmmod ile kaldırıldığında yapılacaklar
static void proc_test_exit(void)
{
        printk("proc test module de-init: start");
        remove_proc_entry(PROC_TEST_CONFIG_FILE, proc_test_dir);
        remove_proc_entry(PROC_TEST_DIR, NULL);
        printk("proc test module de-init: iend");
}

module_init(proc_test_init);
module_exit(proc_test_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("VeteranStudents");

