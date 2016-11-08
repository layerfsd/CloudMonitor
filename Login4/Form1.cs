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
    public partial class MainApp : Form
    {
        public MainApp()
        {
            InitializeComponent();
        }

        private void pictureBox1_Click(object sender, EventArgs e)
        {

        }

        private void Form1_Load(object sender, EventArgs e)
        {
        }

        private void buttonLogin_Click(object sender, EventArgs e)
        {
            string account = textAccount.Text;
            string passwd = textPasswd.Text;
            bool ableLogin = false;

            if (account.Length > 1 && passwd.Length > 1)
            {
                ableLogin = true;   
            }

            if (ableLogin)
            {
                labelStatus.Text = "正在登录中 ...";
            }

            string cmd = "D:\\Dev\\CloudMonitor\\Debug\\CloudMonitor.exe";
            System.Diagnostics.Process.Start(@cmd);
            //this.Close();
            //LoginSuccess form =  new LoginSuccess();
            //form.Show();
            //new LoginSuccess().Show();
        }

        private void textPasswd_TextChanged(object sender, EventArgs e)
        {
            buttonLogin.Enabled = true;
        }

        private void notifyIcon1_MouseDoubleClick(object sender, MouseEventArgs e)
        {

        }

        private void buttonCancel_Click(object sender, EventArgs e)
        {
            textAccount.Clear();
            textPasswd.Clear();
            //this.Close();
        }
    }
}
