namespace Breaknes
{
	partial class FormVirtualFamiController1
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
			button1 = new Button();
			button2 = new Button();
			button3 = new Button();
			button4 = new Button();
			button5 = new Button();
			button6 = new Button();
			button7 = new Button();
			button8 = new Button();
			SuspendLayout();
			// 
			// button1
			// 
			button1.Font = new Font("Segoe UI", 8.25F, FontStyle.Regular, GraphicsUnit.Point);
			button1.Location = new Point(73, 15);
			button1.Name = "button1";
			button1.Size = new Size(33, 40);
			button1.TabIndex = 0;
			button1.Text = "U";
			button1.UseVisualStyleBackColor = true;
			button1.Click += button1_Click;
			// 
			// button2
			// 
			button2.Font = new Font("Segoe UI", 8.25F, FontStyle.Regular, GraphicsUnit.Point);
			button2.Location = new Point(73, 103);
			button2.Name = "button2";
			button2.Size = new Size(33, 40);
			button2.TabIndex = 1;
			button2.Text = "D";
			button2.UseVisualStyleBackColor = true;
			button2.Click += button2_Click;
			// 
			// button3
			// 
			button3.Font = new Font("Segoe UI", 8.25F, FontStyle.Regular, GraphicsUnit.Point);
			button3.Location = new Point(127, 63);
			button3.Name = "button3";
			button3.Size = new Size(40, 33);
			button3.TabIndex = 2;
			button3.Text = "R";
			button3.UseVisualStyleBackColor = true;
			button3.Click += button3_Click;
			// 
			// button4
			// 
			button4.Font = new Font("Segoe UI", 8.25F, FontStyle.Regular, GraphicsUnit.Point);
			button4.Location = new Point(14, 63);
			button4.Name = "button4";
			button4.Size = new Size(40, 33);
			button4.TabIndex = 3;
			button4.Text = "L";
			button4.UseVisualStyleBackColor = true;
			button4.Click += button4_Click;
			// 
			// button5
			// 
			button5.Font = new Font("Segoe UI", 8.25F, FontStyle.Regular, GraphicsUnit.Point);
			button5.Location = new Point(202, 103);
			button5.Name = "button5";
			button5.Size = new Size(66, 33);
			button5.TabIndex = 4;
			button5.Text = "Select";
			button5.UseVisualStyleBackColor = true;
			button5.Click += button5_Click;
			// 
			// button6
			// 
			button6.Font = new Font("Segoe UI", 8.25F, FontStyle.Regular, GraphicsUnit.Point);
			button6.Location = new Point(274, 103);
			button6.Name = "button6";
			button6.Size = new Size(66, 33);
			button6.TabIndex = 5;
			button6.Text = "Start";
			button6.UseVisualStyleBackColor = true;
			button6.Click += button6_Click;
			// 
			// button7
			// 
			button7.Font = new Font("Segoe UI", 8.25F, FontStyle.Regular, GraphicsUnit.Point);
			button7.Location = new Point(385, 81);
			button7.Name = "button7";
			button7.Size = new Size(55, 55);
			button7.TabIndex = 6;
			button7.Text = "B";
			button7.UseVisualStyleBackColor = true;
			button7.Click += button7_Click;
			// 
			// button8
			// 
			button8.Font = new Font("Segoe UI", 8.25F, FontStyle.Regular, GraphicsUnit.Point);
			button8.Location = new Point(446, 81);
			button8.Name = "button8";
			button8.Size = new Size(55, 55);
			button8.TabIndex = 7;
			button8.Text = "A";
			button8.UseVisualStyleBackColor = true;
			button8.Click += button8_Click;
			// 
			// FormVirtualFamiController1
			// 
			AutoScaleDimensions = new SizeF(7F, 15F);
			AutoScaleMode = AutoScaleMode.Font;
			ClientSize = new Size(517, 157);
			Controls.Add(button8);
			Controls.Add(button7);
			Controls.Add(button6);
			Controls.Add(button5);
			Controls.Add(button4);
			Controls.Add(button3);
			Controls.Add(button2);
			Controls.Add(button1);
			Name = "FormVirtualFamiController1";
			Text = "Virtual Famicom Controller (Port1)";
			ResumeLayout(false);
		}

		#endregion

		private Button button1;
		private Button button2;
		private Button button3;
		private Button button4;
		private Button button5;
		private Button button6;
		private Button button7;
		private Button button8;
	}
}