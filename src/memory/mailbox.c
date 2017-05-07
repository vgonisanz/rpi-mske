#include <memory/mailbox.h>

#include <common/io.h>

#include <memory/barrier.h>
#include <memory/memory.h>

static volatile mske_mailbox* const mailbox0 = (mske_mailbox*)MAILBOX0_ADDRESS;

unsigned int read_mailbox(unsigned int channel)
{
  /* default data error */
  unsigned int data = 0xffffffff;
  unsigned int count = 0;

  /* Check if channel is no greater than 4 bits */
  if(channel > 15)
  {
    printk("Error read mailbox: Cannot read more than 15 channels, given: %d.\n", channel);
    return data;
  }

	/* Repeat until match the channel number desired. If nothing recieved, it eventually give up and returns 0xffffffff */
	while(1)
	{
    /* Read the status register until the empty flag is not set, wait for the mailbox to be available to read */
		while (mailbox0->status & MAILBOX_EMPTY)
		{
			/* Need to check if this is the right thing to do */
			flush_cache();

			/* This is an arbitrary large number */
			if(count++ > READ_MAILBOX_TIMEOUT )
			{
        printk("Error read mailbox: empty too much time.\n");
				return data;
			}
		}
		/* Read data from the read register */
		data_mb();
		data = mailbox0->read;
		data_mb();

    /* Check if is the channel number desired */
		if ((data & 0xF) == channel)
    {
      //data >>= 4; /* TODO check if correct --> The upper 28 bits are the returned data */
			return data;
    }
	}
}

int write_mailbox(unsigned int channel, unsigned int data)
{
  /* default data error */
  unsigned int result = -1;

  /* Check if data override channel bits */
  //if(data & 0x0f)
  //{
  //printk("Error write mailbox: data override channel bits.\n");
  //  return result;
  //}

  /* Check if channel is no greater than 4 bits */
  if(channel > 15)
  {
    printk("Error write mailbox: Cannot read more than 15 channels, given: %d.\n", channel);
    return result;
  }

  /* Read the status register until the full flag is not set, wait for the mailbox to be available to write */
  data_mb();
  while( mailbox0->status & MAILBOX_FULL )
  {
    data_mb();
  }

  /* Write the data (shifted into the upper 28 bits) and channel (in the lower four bits)  */
  data_mb();
  mailbox0->write = data | channel;
  result = 0;
  return result;
}
