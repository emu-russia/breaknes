namespace PPUPlayer
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
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(FormAbout));
			pictureBox1 = new System.Windows.Forms.PictureBox();
			label1 = new System.Windows.Forms.Label();
			button1 = new System.Windows.Forms.Button();
			backgroundWorker1 = new System.ComponentModel.BackgroundWorker();
			((System.ComponentModel.ISupportInitialize)pictureBox1).BeginInit();
			SuspendLayout();
			// 
			// pictureBox1
			// 
			pictureBox1.Image = (System.Drawing.Image)resources.GetObject("pictureBox1.Image");
			pictureBox1.Location = new System.Drawing.Point(8, 11);
			pictureBox1.Name = "pictureBox1";
			pictureBox1.Size = new System.Drawing.Size(532, 171);
			pictureBox1.SizeMode = System.Windows.Forms.PictureBoxSizeMode.Zoom;
			pictureBox1.TabIndex = 0;
			pictureBox1.TabStop = false;
			pictureBox1.Click += pictureBox1_Click;
			// 
			// label1
			// 
			label1.AutoSize = true;
			label1.Font = new System.Drawing.Font("Segoe UI Semibold", 12F, System.Drawing.FontStyle.Bold | System.Drawing.FontStyle.Italic, System.Drawing.GraphicsUnit.Point);
			label1.Location = new System.Drawing.Point(11, 205);
			label1.Name = "label1";
			label1.Size = new System.Drawing.Size(480, 42);
			label1.TabIndex = 1;
			label1.Text = "Functional Ricoh 2C02/2C07 and compatible PPU simulator, v. 2.7\r\n© 2023, emu-russia";
			// 
			// button1
			// 
			button1.Anchor = System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right;
			button1.Location = new System.Drawing.Point(515, 215);
			button1.Name = "button1";
			button1.Size = new System.Drawing.Size(97, 33);
			button1.TabIndex = 2;
			button1.Text = "Close";
			button1.UseVisualStyleBackColor = true;
			button1.Click += button1_Click;
			// 
			// backgroundWorker1
			// 
			backgroundWorker1.DoWork += backgroundWorker1_DoWork;
			// 
			// FormAbout
			// 
			AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
			AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			ClientSize = new System.Drawing.Size(624, 260);
			Controls.Add(button1);
			Controls.Add(label1);
			Controls.Add(pictureBox1);
			KeyPreview = true;
			Name = "FormAbout";
			StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			Text = "About PPU Player";
			KeyUp += FormAbout_KeyUp;
			((System.ComponentModel.ISupportInitialize)pictureBox1).EndInit();
			ResumeLayout(false);
			PerformLayout();
		}

		#endregion

		private System.Windows.Forms.PictureBox pictureBox1;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Button button1;
		private System.ComponentModel.BackgroundWorker backgroundWorker1;
	}
}