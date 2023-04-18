



void lnRunTimeInit()
{

}

extern "C" void deadEnd()
{
  while(1)
    {
        __asm__("nop");
    }
}



void bootloader(void)
{
  deadEnd();

}

extern "C" void start_c()
{
    deadEnd();
}