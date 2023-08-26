namespace Breaknes
{
	partial class FormIOAddDevice
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
			label1 = new Label();
			textBox1 = new TextBox();
			label2 = new Label();
			comboBox1 = new ComboBox();
			button1 = new Button();
			SuspendLayout();
			// 
			// label1
			// 
			label1.AutoSize = true;
			label1.Location = new Point(23, 28);
			label1.Name = "label1";
			label1.Size = new Size(39, 15);
			label1.TabIndex = 0;
			label1.Text = "Name";
			// 
			// textBox1
			// 
			textBox1.Location = new Point(91, 25);
			textBox1.Name = "textBox1";
			textBox1.Size = new Size(255, 23);
			textBox1.TabIndex = 1;
			// 
			// label2
			// 
			label2.AutoSize = true;
			label2.Location = new Point(23, 73);
			label2.Name = "label2";
			label2.Size = new Size(53, 15);
			label2.TabIndex = 2;
			label2.Text = "DeviceID";
			// 
			// comboBox1
			// 
			comboBox1.DropDownStyle = ComboBoxStyle.DropDownList;
			comboBox1.FormattingEnabled = true;
			comboBox1.Items.AddRange(new object[] { "Famicom Controller Port1 (0x00000001)", "Famicom Controller Port2 (0x00000002)", "NES Controller (0x00000003)", "Dendy Turbo Controller (0x00000004)", "Virtual Famicom Controller Port1 (0x00010001)", "Virtual Famicom Controller Port2 (0x00010002)", "Virtual NES Controller (0x00010003)", "Virtual Dendy Turbo Controller (0x00010004)" });
			comboBox1.Location = new Point(91, 70);
			comboBox1.Name = "comboBox1";
			comboBox1.Size = new Size(255, 23);
			comboBox1.TabIndex = 3;
			// 
			// button1
			// 
			button1.Location = new Point(262, 115);
			button1.Name = "button1";
			button1.Size = new Size(84, 31);
			button1.TabIndex = 4;
			button1.Text = "OK";
			button1.UseVisualStyleBackColor = true;
			button1.Click += button1_Click;
			// 
			// FormIOAddDevice
			// 
			AutoScaleDimensions = new SizeF(7F, 15F);
			AutoScaleMode = AutoScaleMode.Font;
			ClientSize = new Size(359, 158);
			Controls.Add(button1);
			Controls.Add(comboBox1);
			Controls.Add(label2);
			Controls.Add(textBox1);
			Controls.Add(label1);
			Name = "FormIOAddDevice";
			StartPosition = FormStartPosition.CenterParent;
			Text = "Add IO Device";
			ResumeLayout(false);
			PerformLayout();
		}

		#endregion

		private Label label1;
		private TextBox textBox1;
		private Label label2;
		private ComboBox comboBox1;
		private Button button1;
	}
}