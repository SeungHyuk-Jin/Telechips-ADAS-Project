#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/gpio/consumer.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/wait.h>
#include <linux/interrupt.h>
#include <linux/poll.h>
#include <linux/jiffies.h>

#define DEVICE_NAME "buttons_dev"
#define DEBOUNCE_TIME_MS 100

static struct gpio_desc *button_gpios[4];
static int irq_numbers[4];
static int major;

static DECLARE_WAIT_QUEUE_HEAD(wait_queue);
static int button_pressed = 0;
static int pressed_button_id = -1;

/* 디바운스용 마지막 처리 시간 저장 */
static unsigned long last_pressed_jiffies[4] = {0};

static irqreturn_t button_isr(int irq, void *dev_id)
{
    int id = *(int *)dev_id; // 1 ~ 4
    unsigned long now = jiffies;

    // 디바운스: 이전 처리 후 50ms 이내면 무시
    if (time_before(now, last_pressed_jiffies[id - 1] + msecs_to_jiffies(DEBOUNCE_TIME_MS))) {
        return IRQ_HANDLED;
    }

    last_pressed_jiffies[id - 1] = now;

    pressed_button_id = id;
    button_pressed = 1;

    pr_info("Button %d interrupt triggered\n", id);

    wake_up_interruptible(&wait_queue);
    return IRQ_HANDLED;
}

static int buttons_open(struct inode *inode, struct file *file)
{
    return 0;
}

static int buttons_release(struct inode *inode, struct file *file)
{
    return 0;
}

static ssize_t buttons_read(struct file *file, char __user *buf, size_t len, loff_t *offset)
{
    wait_event_interruptible(wait_queue, button_pressed != 0);

    char msg[32];
    int msg_len = snprintf(msg, sizeof(msg), "button %d pressed\n", pressed_button_id);

    button_pressed = 0;

    if (len < msg_len)
        return -EINVAL;

    if (copy_to_user(buf, msg, msg_len))
        return -EFAULT;

    return msg_len;
}

static __poll_t buttons_poll(struct file *file, struct poll_table_struct *wait)
{
    poll_wait(file, &wait_queue, wait);
    if (button_pressed)
        return POLLIN | POLLRDNORM;
    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = buttons_open,
    .release = buttons_release,
    .read = buttons_read,
    .poll = buttons_poll,
};

static int buttons_probe(struct platform_device *pdev)
{
    int ret;
    static int ids[4] = {1, 2, 3, 4}; // ISR에 넘길 ID

    pr_info("Buttons driver probed\n");

    // 버튼 4개 GPIO 가져오기
    button_gpios[0] = devm_gpiod_get(&pdev->dev, "button1", GPIOD_IN);
    button_gpios[1] = devm_gpiod_get(&pdev->dev, "button2", GPIOD_IN);
    button_gpios[2] = devm_gpiod_get(&pdev->dev, "button3", GPIOD_IN);
    button_gpios[3] = devm_gpiod_get(&pdev->dev, "button4", GPIOD_IN);

    for (int i = 0; i < 4; i++) {
        if (IS_ERR(button_gpios[i])) {
            pr_err("Failed to get button %d GPIO\n", i + 1);
            return PTR_ERR(button_gpios[i]);
        }

        irq_numbers[i] = gpiod_to_irq(button_gpios[i]);
        if (irq_numbers[i] < 0) {
            pr_err("Failed to get IRQ number for button %d\n", i + 1);
            return irq_numbers[i];
        }

        ret = devm_request_irq(&pdev->dev, irq_numbers[i], button_isr,
                               IRQF_TRIGGER_FALLING,
                               DEVICE_NAME, &ids[i]);
        if (ret) {
            pr_err("Failed to request IRQ for button %d\n", i + 1);
            return ret;
        }
    }

    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0) {
        pr_err("Failed to register chrdev\n");
        return major;
    }

    pr_info("/dev/%s created with major %d\n", DEVICE_NAME, major);

    return 0;
}

static void buttons_remove(struct platform_device *pdev)
{
    unregister_chrdev(major, DEVICE_NAME);
    pr_info("Buttons driver removed\n");
}

static const struct of_device_id buttons_of_match[] = {
    { .compatible = "my,buttons", },
    { },
};
MODULE_DEVICE_TABLE(of, buttons_of_match);

static struct platform_driver buttons_driver = {
    .probe = buttons_probe,
    .remove = buttons_remove,
    .driver = {
        .name = "buttons",
        .of_match_table = buttons_of_match,
    },
};

module_platform_driver(buttons_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("YourName");
MODULE_DESCRIPTION("Platform driver for 4 buttons with interrupt, char device and software debounce");
