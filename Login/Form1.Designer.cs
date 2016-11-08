namespace Login4
{
    partial class MainApp
    {
        /// <summary>
        /// 必需的设计器变量。
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// 清理所有正在使用的资源。
        /// </summary>
        /// <param name="disposing">如果应释放托管资源，为 true；否则为 false。</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows 窗体设计器生成的代码

        /// <summary>
        /// 设计器支持所需的方法 - 不要修改
        /// 使用代码编辑器修改此方法的内容。
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            this.buttonCancel = new System.Windows.Forms.Button();
            this.buttonLogin = new System.Windows.Forms.Button();
            this.labelAccount = new System.Windows.Forms.Label();
            this.labelPasswd = new System.Windows.Forms.Label();
            this.textAccount = new System.Windows.Forms.TextBox();
            this.textPasswd = new System.Windows.Forms.TextBox();
            this.labelStatus = new System.Windows.Forms.Label();
            this.notifyIcon1 = new System.Windows.Forms.NotifyIcon(this.components);
            this.buttonChangepd = new System.Windows.Forms.Button();
            this.buttonContactUs = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // buttonCancel
            // 
            this.buttonCancel.Location = new System.Drawing.Point(112, 191);
            this.buttonCancel.Name = "buttonCancel";
            this.buttonCancel.Size = new System.Drawing.Size(75, 23);
            this.buttonCancel.TabIndex = 0;
            this.buttonCancel.Text = "清空输入";
            this.buttonCancel.UseVisualStyleBackColor = true;
            this.buttonCancel.Click += new System.EventHandler(this.buttonCancel_Click);
            // 
            // buttonLogin
            // 
            this.buttonLogin.Enabled = false;
            this.buttonLogin.Location = new System.Drawing.Point(256, 191);
            this.buttonLogin.Name = "buttonLogin";
            this.buttonLogin.Size = new System.Drawing.Size(75, 23);
            this.buttonLogin.TabIndex = 1;
            this.buttonLogin.Text = "登录";
            this.buttonLogin.UseVisualStyleBackColor = true;
            this.buttonLogin.Click += new System.EventHandler(this.buttonLogin_Click);
            // 
            // labelAccount
            // 
            this.labelAccount.AutoSize = true;
            this.labelAccount.Font = new System.Drawing.Font("宋体", 10.5F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.labelAccount.Location = new System.Drawing.Point(49, 74);
            this.labelAccount.Name = "labelAccount";
            this.labelAccount.Size = new System.Drawing.Size(35, 14);
            this.labelAccount.TabIndex = 2;
            this.labelAccount.Text = "账号";
            // 
            // labelPasswd
            // 
            this.labelPasswd.AutoSize = true;
            this.labelPasswd.Font = new System.Drawing.Font("宋体", 10.5F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.labelPasswd.Location = new System.Drawing.Point(49, 132);
            this.labelPasswd.Name = "labelPasswd";
            this.labelPasswd.Size = new System.Drawing.Size(35, 14);
            this.labelPasswd.TabIndex = 3;
            this.labelPasswd.Text = "密码";
            // 
            // textAccount
            // 
            this.textAccount.Location = new System.Drawing.Point(112, 73);
            this.textAccount.Name = "textAccount";
            this.textAccount.Size = new System.Drawing.Size(100, 21);
            this.textAccount.TabIndex = 4;
            // 
            // textPasswd
            // 
            this.textPasswd.Location = new System.Drawing.Point(112, 125);
            this.textPasswd.Name = "textPasswd";
            this.textPasswd.PasswordChar = '*';
            this.textPasswd.Size = new System.Drawing.Size(100, 21);
            this.textPasswd.TabIndex = 5;
            this.textPasswd.TextChanged += new System.EventHandler(this.textPasswd_TextChanged);
            // 
            // labelStatus
            // 
            this.labelStatus.AutoSize = true;
            this.labelStatus.BackColor = System.Drawing.SystemColors.ActiveCaption;
            this.labelStatus.Font = new System.Drawing.Font("宋体", 10.5F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.labelStatus.Location = new System.Drawing.Point(91, 25);
            this.labelStatus.Name = "labelStatus";
            this.labelStatus.Size = new System.Drawing.Size(112, 14);
            this.labelStatus.TabIndex = 6;
            this.labelStatus.Text = "当前状态:  离线";
            // 
            // notifyIcon1
            // 
            this.notifyIcon1.Text = "notifyIcon1";
            this.notifyIcon1.Visible = true;
            this.notifyIcon1.MouseDoubleClick += new System.Windows.Forms.MouseEventHandler(this.notifyIcon1_MouseDoubleClick);
            // 
            // buttonChangepd
            // 
            this.buttonChangepd.Location = new System.Drawing.Point(256, 73);
            this.buttonChangepd.Name = "buttonChangepd";
            this.buttonChangepd.Size = new System.Drawing.Size(75, 23);
            this.buttonChangepd.TabIndex = 7;
            this.buttonChangepd.Text = "更改密码";
            this.buttonChangepd.UseVisualStyleBackColor = true;
            // 
            // buttonContactUs
            // 
            this.buttonContactUs.Location = new System.Drawing.Point(256, 123);
            this.buttonContactUs.Name = "buttonContactUs";
            this.buttonContactUs.Size = new System.Drawing.Size(75, 23);
            this.buttonContactUs.TabIndex = 8;
            this.buttonContactUs.Text = "联系我们";
            this.buttonContactUs.UseVisualStyleBackColor = true;
            // 
            // MainApp
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(363, 263);
            this.Controls.Add(this.buttonContactUs);
            this.Controls.Add(this.buttonChangepd);
            this.Controls.Add(this.labelStatus);
            this.Controls.Add(this.textPasswd);
            this.Controls.Add(this.textAccount);
            this.Controls.Add(this.labelPasswd);
            this.Controls.Add(this.labelAccount);
            this.Controls.Add(this.buttonLogin);
            this.Controls.Add(this.buttonCancel);
            this.MaximizeBox = false;
            this.Name = "MainApp";
            this.ShowIcon = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "云控平台-登录";
            this.Load += new System.EventHandler(this.Form1_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button buttonCancel;
        private System.Windows.Forms.Button buttonLogin;
        private System.Windows.Forms.Label labelAccount;
        private System.Windows.Forms.Label labelPasswd;
        private System.Windows.Forms.TextBox textAccount;
        private System.Windows.Forms.TextBox textPasswd;
        private System.Windows.Forms.Label labelStatus;
        private System.Windows.Forms.NotifyIcon notifyIcon1;
        private System.Windows.Forms.Button buttonChangepd;
        private System.Windows.Forms.Button buttonContactUs;
    }
}

