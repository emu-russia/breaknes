namespace Breaknes
{
	partial class FormVirtualFamiController2
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
			checkBox8 = new CheckBox();
			checkBox7 = new CheckBox();
			checkBox4 = new CheckBox();
			checkBox3 = new CheckBox();
			checkBox2 = new CheckBox();
			checkBox1 = new CheckBox();
			trackBar1 = new TrackBar();
			label1 = new Label();
			label2 = new Label();
			trackBar2 = new TrackBar();
			((System.ComponentModel.ISupportInitialize)trackBar1).BeginInit();
			((System.ComponentModel.ISupportInitialize)trackBar2).BeginInit();
			SuspendLayout();
			// 
			// checkBox8
			// 
			checkBox8.Appearance = Appearance.Button;
			checkBox8.AutoSize = true;
			checkBox8.Font = new Font("Segoe UI", 14.25F, FontStyle.Regular, GraphicsUnit.Point);
			checkBox8.Location = new Point(303, 112);
			checkBox8.Name = "checkBox8";
			checkBox8.Size = new Size(34, 35);
			checkBox8.TabIndex = 23;
			checkBox8.Text = "A";
			checkBox8.UseVisualStyleBackColor = true;
			// 
			// checkBox7
			// 
			checkBox7.Appearance = Appearance.Button;
			checkBox7.AutoSize = true;
			checkBox7.Font = new Font("Segoe UI", 14.25F, FontStyle.Regular, GraphicsUnit.Point);
			checkBox7.Location = new Point(264, 112);
			checkBox7.Name = "checkBox7";
			checkBox7.Size = new Size(33, 35);
			checkBox7.TabIndex = 22;
			checkBox7.Text = "B";
			checkBox7.UseVisualStyleBackColor = true;
			// 
			// checkBox4
			// 
			checkBox4.Appearance = Appearance.Button;
			checkBox4.AutoSize = true;
			checkBox4.Location = new Point(53, 122);
			checkBox4.Name = "checkBox4";
			checkBox4.Size = new Size(25, 25);
			checkBox4.TabIndex = 19;
			checkBox4.Text = "D";
			checkBox4.UseVisualStyleBackColor = true;
			// 
			// checkBox3
			// 
			checkBox3.Appearance = Appearance.Button;
			checkBox3.AutoSize = true;
			checkBox3.Location = new Point(90, 94);
			checkBox3.Name = "checkBox3";
			checkBox3.Size = new Size(24, 25);
			checkBox3.TabIndex = 18;
			checkBox3.Text = "R";
			checkBox3.UseVisualStyleBackColor = true;
			// 
			// checkBox2
			// 
			checkBox2.Appearance = Appearance.Button;
			checkBox2.AutoSize = true;
			checkBox2.Location = new Point(19, 94);
			checkBox2.Name = "checkBox2";
			checkBox2.Size = new Size(23, 25);
			checkBox2.TabIndex = 17;
			checkBox2.Text = "L";
			checkBox2.UseVisualStyleBackColor = true;
			// 
			// checkBox1
			// 
			checkBox1.Appearance = Appearance.Button;
			checkBox1.AutoSize = true;
			checkBox1.Location = new Point(53, 63);
			checkBox1.Name = "checkBox1";
			checkBox1.Size = new Size(25, 25);
			checkBox1.TabIndex = 16;
			checkBox1.Text = "U";
			checkBox1.UseVisualStyleBackColor = true;
			// 
			// trackBar1
			// 
			trackBar1.Location = new Point(12, 12);
			trackBar1.Maximum = 255;
			trackBar1.Name = "trackBar1";
			trackBar1.Size = new Size(104, 45);
			trackBar1.TabIndex = 24;
			// 
			// label1
			// 
			label1.AutoSize = true;
			label1.Location = new Point(19, 42);
			label1.Name = "label1";
			label1.Size = new Size(47, 15);
			label1.TabIndex = 25;
			label1.Text = "Volume";
			// 
			// label2
			// 
			label2.AutoSize = true;
			label2.Location = new Point(153, 94);
			label2.Name = "label2";
			label2.Size = new Size(57, 15);
			label2.TabIndex = 27;
			label2.Text = "Mic Level";
			// 
			// trackBar2
			// 
			trackBar2.Location = new Point(144, 63);
			trackBar2.Maximum = 255;
			trackBar2.Name = "trackBar2";
			trackBar2.Size = new Size(104, 45);
			trackBar2.TabIndex = 26;
			// 
			// FormVirtualFamiController2
			// 
			AutoScaleDimensions = new SizeF(7F, 15F);
			AutoScaleMode = AutoScaleMode.Font;
			ClientSize = new Size(355, 159);
			Controls.Add(label2);
			Controls.Add(trackBar2);
			Controls.Add(label1);
			Controls.Add(trackBar1);
			Controls.Add(checkBox8);
			Controls.Add(checkBox7);
			Controls.Add(checkBox4);
			Controls.Add(checkBox3);
			Controls.Add(checkBox2);
			Controls.Add(checkBox1);
			Name = "FormVirtualFamiController2";
			Text = "Virtual Famicom Controller (Port2)";
			((System.ComponentModel.ISupportInitialize)trackBar1).EndInit();
			((System.ComponentModel.ISupportInitialize)trackBar2).EndInit();
			ResumeLayout(false);
			PerformLayout();
		}

		#endregion

		private CheckBox checkBox8;
		private CheckBox checkBox7;
		private CheckBox checkBox4;
		private CheckBox checkBox3;
		private CheckBox checkBox2;
		private CheckBox checkBox1;
		private TrackBar trackBar1;
		private Label label1;
		private Label label2;
		private TrackBar trackBar2;
	}
}