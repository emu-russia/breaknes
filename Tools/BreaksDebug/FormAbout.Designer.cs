
namespace BreaksDebug
{
	partial class FormAbout
	{
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.IContainer components = null;

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		/// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
		protected override void Dispose(bool disposing)
		{
			if (disposing && (components != null))
			{
				components.Dispose();
			}
			base.Dispose(disposing);
		}

		#region Windows Form Designer generated code

		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			pictureBox1 = new PictureBox();
			label1 = new Label();
			linkLabel1 = new LinkLabel();
			button1 = new Button();
			((System.ComponentModel.ISupportInitialize)pictureBox1).BeginInit();
			SuspendLayout();
			// 
			// pictureBox1
			// 
			pictureBox1.BackgroundImage = Properties.Resources.hand_holding_hammer_icon_png;
			pictureBox1.BackgroundImageLayout = ImageLayout.Zoom;
			pictureBox1.Location = new Point(17, 20);
			pictureBox1.Margin = new Padding(5, 4, 5, 4);
			pictureBox1.Name = "pictureBox1";
			pictureBox1.Size = new Size(126, 112);
			pictureBox1.TabIndex = 0;
			pictureBox1.TabStop = false;
			// 
			// label1
			// 
			label1.AutoSize = true;
			label1.Font = new Font("Microsoft Sans Serif", 9.75F, FontStyle.Bold);
			label1.Location = new Point(153, 20);
			label1.Margin = new Padding(5, 0, 5, 0);
			label1.Name = "label1";
			label1.Size = new Size(211, 40);
			label1.TabIndex = 1;
			label1.Text = "Breaks Debugger, v. 2.4\r\n© 2026, emu-russia";
			// 
			// linkLabel1
			// 
			linkLabel1.AutoSize = true;
			linkLabel1.Location = new Point(153, 71);
			linkLabel1.Margin = new Padding(5, 0, 5, 0);
			linkLabel1.Name = "linkLabel1";
			linkLabel1.Size = new Size(259, 20);
			linkLabel1.TabIndex = 2;
			linkLabel1.TabStop = true;
			linkLabel1.Text = "https://github.com/emu-russia/breaks";
			// 
			// button1
			// 
			button1.Anchor = AnchorStyles.Bottom | AnchorStyles.Right;
			button1.Location = new Point(488, 122);
			button1.Margin = new Padding(5, 4, 5, 4);
			button1.Name = "button1";
			button1.Size = new Size(126, 47);
			button1.TabIndex = 3;
			button1.Text = "Close";
			button1.UseVisualStyleBackColor = true;
			button1.Click += button1_Click;
			// 
			// FormAbout
			// 
			AutoScaleDimensions = new SizeF(8F, 20F);
			AutoScaleMode = AutoScaleMode.Font;
			ClientSize = new Size(628, 182);
			Controls.Add(button1);
			Controls.Add(linkLabel1);
			Controls.Add(label1);
			Controls.Add(pictureBox1);
			KeyPreview = true;
			Margin = new Padding(5, 4, 5, 4);
			MinimumSize = new Size(554, 223);
			Name = "FormAbout";
			StartPosition = FormStartPosition.CenterParent;
			Text = "About";
			KeyDown += FormAbout_KeyDown;
			((System.ComponentModel.ISupportInitialize)pictureBox1).EndInit();
			ResumeLayout(false);
			PerformLayout();
		}

		#endregion

		private System.Windows.Forms.PictureBox pictureBox1;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.LinkLabel linkLabel1;
		private System.Windows.Forms.Button button1;
	}
}