using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace Login4
{
    public partial class LoginSuccess : Form
    {
        public LoginSuccess()
        {
            InitializeComponent();
        }

        private void timerStart_Tick(object sender, EventArgs e)
        {
    
        }

        private void LoginSuccess_Load(object sender, EventArgs e)
        {
            int Heightone = System.Windows.Forms.Screen.PrimaryScreen.Bounds.Height;         //获取屏幕的高度
            int Heighttwo = System.Windows.Forms.Screen.PrimaryScreen.WorkingArea.Height;   //获取工作区的高度
            int screenX = System.Windows.Forms.Screen.PrimaryScreen.Bounds.Width;      //获取屏幕的宽度

            int renwu = Heightone - Heighttwo;          //屏幕的高度减去工作区的高度，得到任务栏的高度，只所以获取任务栏的高度，是由于一些时候任务栏的高度不固定。避免窗体被任务栏遮挡住
            this.Top = Heightone - this.Height - renwu;     //距离上边的距离＝屏幕的高度－窗体的高度－任务栏的高度
            this.Left = screenX - this.Width;           //距离左边的距离＝屏幕的宽度－窗体的宽度

            this.labelStatus.Text = "WWWWWWWWWWWWWWWWWW";

            //this.Opacity = 0;     //设置窗体的不透明度为0
            //this.Close();
        }

        private void labelUseName_Click(object sender, EventArgs e)
        {

        }
    }
}
