using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Solar_System_Simulation
{
    public partial class GUI : Form
    {

        public int[] XValue = { 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000 };
        public int[] YValue = { 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000 };

        public int GUI_Width    = 1600;
        public int GUI_Height   = 900;

        int NumBodies = 2;

        private int StartCollect, Index = 0, Receive_Done = 0;

        int[] RecData   = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        int[] PixelData = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

        int[] TimeData = { 0, 0, 0 };
        int Time_Receive_Done = 0, TimeIndex = 0, StartCollectTime = 0;

        int EarthNum = 1, EarthDays = 0, EarthCount = 0;

        //public int SunX, SunXPrev, SunXTemp, EarthX, EarthXPrev, EarthXTemp, MoonX, MoonXPrev, MoonXTemp, VenusX,VenusXPrev, VenusXTemp, MarsX,MarsXPrev,MarsXTemp, JupiterX,JupiterXPrev,JupiterXTemp, SaturnX,SaturnXTemp,SaturnXPrev, UranusX,UranusXTemp,UranusXPrev, NeptuneX,NeptuneXPrev,NeptuneXTemp;
        //public int SunY, SunYPrev, SunYTemp, EarthY, EarthYPrev, EarthYTemp, MoonY, MoonYPrev, MoonYTemp, VenusY, VenusYPrev, VenusYTemp, MarsY, MarsYPrev, MarsYTemp, JupiterY, JupiterYPrev, JupiterYTemp, SaturnY, SaturnYTemp, SaturnYPrev, UranusY, UranusYTemp, UranusYPrev, NeptuneY, NeptuneYPrev, NeptuneYTemp;
        

        public GUI()
        {
            InitializeComponent();

            XValue[0] = GUI_Width / 2;      //SunX
            YValue[0] = GUI_Height / 2;     //SunY

            XValue[1] = XValue[0] + 30;      //MercuryX
            YValue[1] = YValue[0] + 0;       //MercuryY

            if (NumBodies > 2)
            {
                XValue[2] = XValue[0] + 100;     //VenusX
                YValue[2] = YValue[0];          //VenusY
            }

            if (NumBodies > 3)
            {
                XValue[3] = XValue[0] + 202;      //EarthX
                YValue[3] = YValue[0] + 0;        //EarthX
            }


            if (NumBodies > 4)
            {
                XValue[4] = XValue[0] + 420;     //MarsX
                YValue[4] = YValue[0];            //MarsY
            }

            if (NumBodies > 5)
            {
                XValue[5] = XValue[0] + 460;       //JupiterX
                YValue[5] = YValue[0];             //JupiterY
            }

            if (NumBodies > 6)
            {
                XValue[6] = XValue[0] + 550;      //SaturnX
                YValue[6] = YValue[0];            //SaturnY
            }

            if (NumBodies > 7)
            {
                XValue[7] = XValue[0] + 670;      //UranusX
                YValue[7] = YValue[0];            //UranusY
            }

            if (NumBodies > 8)
            {
                XValue[8] = XValue[0] + 730;      //NeptuneX
                YValue[8] = YValue[0];            //NeptuneY
            }


            if (!SerialPort.IsOpen)
            {
                SerialPort.PortName = "COM4";
                SerialPort.Open();
            }

            SerialPort.DiscardInBuffer();


            richTextBox2.Clear();
            richTextBox2.AppendText(EarthNum.ToString());

            richTextBox4.Clear();
            richTextBox4.AppendText(EarthDays.ToString());
        }

        private void GUI_Paint(object sender, PaintEventArgs e)
        {

            e.Graphics.DrawImage(new    Bitmap("sun.ico"),      XValue[0], YValue[0], 40, 40);
            e.Graphics.DrawImage(new    Bitmap("Mercury.ico"),  XValue[1], YValue[1], 15, 15);
            e.Graphics.DrawImage(new    Bitmap("Earth.ico"),    XValue[2], YValue[2], 25, 25);
            e.Graphics.DrawImage(new    Bitmap("mars.ico"),     XValue[3], YValue[3], 25, 25);
            e.Graphics.DrawImage(new    Bitmap("venus.png"),    XValue[4], YValue[4], 27, 27);
            e.Graphics.DrawImage(new    Bitmap("jupiter.ico"),  XValue[5], YValue[5], 35, 35);
            e.Graphics.DrawImage(new    Bitmap("saturn.ico"),   XValue[6], YValue[6], 30, 30);
            e.Graphics.DrawImage(new    Bitmap("uranus.ico"),   XValue[7], YValue[7], 30, 30);
            e.Graphics.DrawImage(new    Bitmap("neptune.ico"),  XValue[8], YValue[8], 25, 25);
        }

#if NEVER
        private void GUI_Resize_1(object sender, EventArgs e)
        {
            Control control = (Control)sender;

            GUI_Height = control.Size.Height;
            GUI_Width = control.Size.Width;

            SunX = GUI_Width / 2;
            SunY = GUI_Height / 2;
        }
#endif
        private void timer1_Tick(object sender, EventArgs e)
        {

            int i = 0, k = 0;

            if (Receive_Done == 1)
            {
                PixelData = RecData;

                k = 0;
                for (i = 0; i < NumBodies; i++)
                {
                    XValue[i] = ((PixelData[k] * 100) + PixelData[k+1]);
                    YValue[i] = ((PixelData[k+2] * 100) + PixelData[k+3]);

                    k = k + 4;
                    
                }

            }

            Receive_Done = 0;
            StartCollect = 0;

            if (Time_Receive_Done == 1)
            {
                EarthDays = TimeData[0] * 100 + TimeData[1];
                EarthNum = TimeData[2];
                
                richTextBox2.Clear();
                richTextBox2.AppendText(EarthNum.ToString());

                richTextBox4.Clear();
                richTextBox4.AppendText(EarthDays.ToString());
                EarthCount++;


                //EarthCount++;
            }




            Invalidate();
        }

        public int ReceivedData;
        private void SerialPort_DataReceived(object sender, System.IO.Ports.SerialDataReceivedEventArgs e)
        {
            ReceivedData = (char)SerialPort.ReadByte();

            if (ReceivedData == 243)
            {
                Index = 0;
                StartCollect = 1;               
            }
            else if (StartCollect == 1)
            {
                RecData[Index] = ReceivedData;
                Index++;
            }

            if (ReceivedData == '@')
            {
                TimeIndex = 0;
                StartCollectTime = 1;
            }
            else if (StartCollectTime == 1)
            {
                TimeData[TimeIndex] = ReceivedData;
                TimeIndex++;
            }


            if (Index > (NumBodies*4-1))
            {
                StartCollect = 0;
                Receive_Done = 1;
                Index = 0;
            }

            if (TimeIndex > 2)
            {
                StartCollectTime = 0;
                Time_Receive_Done = 1;
                TimeIndex=0;
            }

        }

        private void Done_CheckedChanged(object sender, EventArgs e)
        {
            Do_Checked();
        }

        private void Do_Checked()
        {
            bool en = Done.Checked;
            if (en)
            {
                SerialPort.Write("S");
                //timer1.Enabled = true;
            }
            else if (!en)
            {
                SerialPort.Write("E");
                //timer1.Enabled = false;
            }
        }

        private void trackBar1_ValueChanged(object sender, EventArgs e)
        {
            int bar_value = trackBar1.Value;
            if (bar_value == 10)
                timer1.Interval = 10;

            if (bar_value == 9)
                timer1.Interval = 30;

            if (bar_value == 8)
                timer1.Interval = 50;

            if (bar_value == 7)
                timer1.Interval = 70;

            if (bar_value == 6)
                timer1.Interval = 100;

            if (bar_value == 5)
                timer1.Interval = 120;

            if (bar_value == 4)
                timer1.Interval = 140;

            if (bar_value == 3)
                timer1.Interval = 160;

            if (bar_value == 2)
                timer1.Interval = 180;

            if (bar_value == 1)
                timer1.Interval = 200;

            if (bar_value == 0)
                timer1.Interval = 220;

        }


        private void GUI_Load(object sender, EventArgs e)
        {

        }


        private void textBox1_TextChanged(object sender, EventArgs e)
        {
            timer1.Interval = Convert.ToInt16(textBox1.Text);
        }

        private void button1_Click(object sender, EventArgs e)
        {
            int x;
            x = Convert.ToInt16(textBox1.Text);
            if(x != 0)
                timer1.Interval = Convert.ToInt16(textBox1.Text);
        }



    }
}
