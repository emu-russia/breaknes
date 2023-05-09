namespace SharpTools
{
	partial class BreakpointProps
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

		#region Component Designer generated code

		/// <summary> 
		/// Required method for Designer support - do not modify 
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			groupBox1 = new GroupBox();
			comboBox3 = new ComboBox();
			label3 = new Label();
			comboBox2 = new ComboBox();
			label2 = new Label();
			comboBox1 = new ComboBox();
			label1 = new Label();
			groupBox2 = new GroupBox();
			textBox1 = new TextBox();
			label5 = new Label();
			comboBox4 = new ComboBox();
			label4 = new Label();
			checkBox2 = new CheckBox();
			checkBox1 = new CheckBox();
			groupBox1.SuspendLayout();
			groupBox2.SuspendLayout();
			SuspendLayout();
			// 
			// groupBox1
			// 
			groupBox1.Controls.Add(comboBox3);
			groupBox1.Controls.Add(label3);
			groupBox1.Controls.Add(comboBox2);
			groupBox1.Controls.Add(label2);
			groupBox1.Controls.Add(comboBox1);
			groupBox1.Controls.Add(label1);
			groupBox1.Location = new Point(13, 13);
			groupBox1.Name = "groupBox1";
			groupBox1.Size = new Size(406, 123);
			groupBox1.TabIndex = 0;
			groupBox1.TabStop = false;
			groupBox1.Text = "Signal";
			// 
			// comboBox3
			// 
			comboBox3.DropDownStyle = ComboBoxStyle.DropDownList;
			comboBox3.FormattingEnabled = true;
			comboBox3.Location = new Point(106, 74);
			comboBox3.Name = "comboBox3";
			comboBox3.Size = new Size(179, 23);
			comboBox3.TabIndex = 5;
			// 
			// label3
			// 
			label3.AutoSize = true;
			label3.Location = new Point(6, 77);
			label3.Name = "label3";
			label3.Size = new Size(39, 15);
			label3.TabIndex = 4;
			label3.Text = "Name";
			// 
			// comboBox2
			// 
			comboBox2.DropDownStyle = ComboBoxStyle.DropDownList;
			comboBox2.FormattingEnabled = true;
			comboBox2.Location = new Point(106, 45);
			comboBox2.Name = "comboBox2";
			comboBox2.Size = new Size(179, 23);
			comboBox2.TabIndex = 3;
			comboBox2.SelectedIndexChanged += comboBox2_SelectedIndexChanged;
			// 
			// label2
			// 
			label2.AutoSize = true;
			label2.Location = new Point(6, 48);
			label2.Name = "label2";
			label2.Size = new Size(55, 15);
			label2.TabIndex = 2;
			label2.Text = "Category";
			// 
			// comboBox1
			// 
			comboBox1.DropDownStyle = ComboBoxStyle.DropDownList;
			comboBox1.FormattingEnabled = true;
			comboBox1.Items.AddRange(new object[] { "Core", "CoreRegs", "APU", "APURegs", "PPU", "PPURegs", "Board", "Cart" });
			comboBox1.Location = new Point(106, 16);
			comboBox1.Name = "comboBox1";
			comboBox1.Size = new Size(179, 23);
			comboBox1.TabIndex = 1;
			comboBox1.SelectedIndexChanged += comboBox1_SelectedIndexChanged;
			// 
			// label1
			// 
			label1.AutoSize = true;
			label1.Location = new Point(6, 19);
			label1.Name = "label1";
			label1.Size = new Size(52, 15);
			label1.TabIndex = 0;
			label1.Text = "InfoType";
			// 
			// groupBox2
			// 
			groupBox2.Controls.Add(textBox1);
			groupBox2.Controls.Add(label5);
			groupBox2.Controls.Add(comboBox4);
			groupBox2.Controls.Add(label4);
			groupBox2.Controls.Add(checkBox2);
			groupBox2.Controls.Add(checkBox1);
			groupBox2.Location = new Point(13, 142);
			groupBox2.Name = "groupBox2";
			groupBox2.Size = new Size(406, 143);
			groupBox2.TabIndex = 1;
			groupBox2.TabStop = false;
			groupBox2.Text = "Conditions";
			// 
			// textBox1
			// 
			textBox1.Location = new Point(106, 100);
			textBox1.Name = "textBox1";
			textBox1.Size = new Size(179, 23);
			textBox1.TabIndex = 9;
			// 
			// label5
			// 
			label5.AutoSize = true;
			label5.Location = new Point(6, 103);
			label5.Name = "label5";
			label5.Size = new Size(71, 15);
			label5.TabIndex = 8;
			label5.Text = "Vector value";
			// 
			// comboBox4
			// 
			comboBox4.DropDownStyle = ComboBoxStyle.DropDownList;
			comboBox4.FormattingEnabled = true;
			comboBox4.Items.AddRange(new object[] { "High", "Low", "Z", "X", "Posedge", "Negedge", "Vector" });
			comboBox4.Location = new Point(106, 70);
			comboBox4.Name = "comboBox4";
			comboBox4.Size = new Size(179, 23);
			comboBox4.TabIndex = 7;
			// 
			// label4
			// 
			label4.AutoSize = true;
			label4.Location = new Point(6, 73);
			label4.Name = "label4";
			label4.Size = new Size(43, 15);
			label4.TabIndex = 6;
			label4.Text = "Trigger";
			// 
			// checkBox2
			// 
			checkBox2.AutoSize = true;
			checkBox2.Location = new Point(6, 47);
			checkBox2.Name = "checkBox2";
			checkBox2.Size = new Size(77, 19);
			checkBox2.TabIndex = 1;
			checkBox2.Text = "Autoclear";
			checkBox2.UseVisualStyleBackColor = true;
			// 
			// checkBox1
			// 
			checkBox1.AutoSize = true;
			checkBox1.Location = new Point(6, 22);
			checkBox1.Name = "checkBox1";
			checkBox1.Size = new Size(68, 19);
			checkBox1.TabIndex = 0;
			checkBox1.Text = "Enabled";
			checkBox1.UseVisualStyleBackColor = true;
			// 
			// BreakpointProps
			// 
			AutoScaleDimensions = new SizeF(7F, 15F);
			AutoScaleMode = AutoScaleMode.Font;
			Controls.Add(groupBox2);
			Controls.Add(groupBox1);
			Name = "BreakpointProps";
			Size = new Size(468, 316);
			groupBox1.ResumeLayout(false);
			groupBox1.PerformLayout();
			groupBox2.ResumeLayout(false);
			groupBox2.PerformLayout();
			ResumeLayout(false);
		}

		#endregion

		private GroupBox groupBox1;
		private ComboBox comboBox3;
		private Label label3;
		private ComboBox comboBox2;
		private Label label2;
		private ComboBox comboBox1;
		private Label label1;
		private GroupBox groupBox2;
		private CheckBox checkBox2;
		private CheckBox checkBox1;
		private TextBox textBox1;
		private Label label5;
		private ComboBox comboBox4;
		private Label label4;
	}
}
