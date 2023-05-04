namespace Breaknes
{
	partial class FormDebugger
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
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(FormDebugger));
			toolStrip1 = new ToolStrip();
			toolStripButton1 = new ToolStripButton();
			toolStripButton2 = new ToolStripButton();
			toolStripButton3 = new ToolStripButton();
			splitContainer1 = new SplitContainer();
			splitContainer3 = new SplitContainer();
			button2 = new Button();
			comboBox2 = new ComboBox();
			propertyGrid1 = new PropertyGrid();
			splitContainer2 = new SplitContainer();
			button1 = new Button();
			comboBox1 = new ComboBox();
			tabControl2 = new TabControl();
			tabPage5 = new TabPage();
			pictureBoxForHuman = new PictureBox();
			tabPage4 = new TabPage();
			hexBox1 = new Be.Windows.Forms.HexBox();
			tabPage1 = new TabPage();
			toolStrip2 = new ToolStrip();
			toolStripButton4 = new ToolStripButton();
			toolStripButton5 = new ToolStripButton();
			toolStripSeparator1 = new ToolStripSeparator();
			toolStripDropDownButton1 = new ToolStripDropDownButton();
			toolStripDropDownButton2 = new ToolStripDropDownButton();
			toolStripDropDownButton3 = new ToolStripDropDownButton();
			wavesControl1 = new WavesControl();
			toolStrip1.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)splitContainer1).BeginInit();
			splitContainer1.Panel1.SuspendLayout();
			splitContainer1.Panel2.SuspendLayout();
			splitContainer1.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)splitContainer3).BeginInit();
			splitContainer3.Panel1.SuspendLayout();
			splitContainer3.Panel2.SuspendLayout();
			splitContainer3.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)splitContainer2).BeginInit();
			splitContainer2.Panel1.SuspendLayout();
			splitContainer2.Panel2.SuspendLayout();
			splitContainer2.SuspendLayout();
			tabControl2.SuspendLayout();
			tabPage5.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)pictureBoxForHuman).BeginInit();
			tabPage4.SuspendLayout();
			tabPage1.SuspendLayout();
			toolStrip2.SuspendLayout();
			SuspendLayout();
			// 
			// toolStrip1
			// 
			toolStrip1.Items.AddRange(new ToolStripItem[] { toolStripButton1, toolStripButton2, toolStripButton3 });
			toolStrip1.Location = new Point(0, 0);
			toolStrip1.Name = "toolStrip1";
			toolStrip1.Size = new Size(1094, 25);
			toolStrip1.TabIndex = 0;
			toolStrip1.Text = "toolStrip1";
			// 
			// toolStripButton1
			// 
			toolStripButton1.DisplayStyle = ToolStripItemDisplayStyle.Text;
			toolStripButton1.Image = (Image)resources.GetObject("toolStripButton1.Image");
			toolStripButton1.ImageTransparentColor = Color.Magenta;
			toolStripButton1.Name = "toolStripButton1";
			toolStripButton1.Size = new Size(42, 22);
			toolStripButton1.Text = "Pause";
			toolStripButton1.Click += toolStripButton1_Click;
			// 
			// toolStripButton2
			// 
			toolStripButton2.DisplayStyle = ToolStripItemDisplayStyle.Text;
			toolStripButton2.Image = (Image)resources.GetObject("toolStripButton2.Image");
			toolStripButton2.ImageTransparentColor = Color.Magenta;
			toolStripButton2.Name = "toolStripButton2";
			toolStripButton2.Size = new Size(63, 22);
			toolStripButton2.Text = "Step (F11)";
			toolStripButton2.Click += toolStripButton2_Click;
			// 
			// toolStripButton3
			// 
			toolStripButton3.DisplayStyle = ToolStripItemDisplayStyle.Text;
			toolStripButton3.Image = (Image)resources.GetObject("toolStripButton3.Image");
			toolStripButton3.ImageTransparentColor = Color.Magenta;
			toolStripButton3.Name = "toolStripButton3";
			toolStripButton3.Size = new Size(55, 22);
			toolStripButton3.Text = "Run (F5)";
			toolStripButton3.Click += toolStripButton3_Click;
			// 
			// splitContainer1
			// 
			splitContainer1.Dock = DockStyle.Fill;
			splitContainer1.Location = new Point(0, 25);
			splitContainer1.Name = "splitContainer1";
			// 
			// splitContainer1.Panel1
			// 
			splitContainer1.Panel1.Controls.Add(splitContainer3);
			// 
			// splitContainer1.Panel2
			// 
			splitContainer1.Panel2.Controls.Add(splitContainer2);
			splitContainer1.Size = new Size(1094, 503);
			splitContainer1.SplitterDistance = 297;
			splitContainer1.TabIndex = 1;
			// 
			// splitContainer3
			// 
			splitContainer3.Dock = DockStyle.Fill;
			splitContainer3.FixedPanel = FixedPanel.Panel1;
			splitContainer3.Location = new Point(0, 0);
			splitContainer3.Name = "splitContainer3";
			splitContainer3.Orientation = Orientation.Horizontal;
			// 
			// splitContainer3.Panel1
			// 
			splitContainer3.Panel1.Controls.Add(button2);
			splitContainer3.Panel1.Controls.Add(comboBox2);
			// 
			// splitContainer3.Panel2
			// 
			splitContainer3.Panel2.Controls.Add(propertyGrid1);
			splitContainer3.Size = new Size(297, 503);
			splitContainer3.SplitterDistance = 42;
			splitContainer3.TabIndex = 1;
			// 
			// button2
			// 
			button2.BackgroundImageLayout = ImageLayout.Zoom;
			button2.Image = Properties.Resources.button_get_debug;
			button2.Location = new Point(248, 5);
			button2.Name = "button2";
			button2.Size = new Size(32, 32);
			button2.TabIndex = 2;
			button2.UseVisualStyleBackColor = true;
			button2.Click += button2_Click;
			// 
			// comboBox2
			// 
			comboBox2.DropDownStyle = ComboBoxStyle.DropDownList;
			comboBox2.FormattingEnabled = true;
			comboBox2.Items.AddRange(new object[] { "Core", "CoreRegs", "APU", "APURegs", "PPU", "PPURegs", "Board", "Cartridge" });
			comboBox2.Location = new Point(8, 9);
			comboBox2.Name = "comboBox2";
			comboBox2.Size = new Size(234, 23);
			comboBox2.TabIndex = 0;
			comboBox2.SelectedIndexChanged += comboBox2_SelectedIndexChanged;
			// 
			// propertyGrid1
			// 
			propertyGrid1.Dock = DockStyle.Fill;
			propertyGrid1.Font = new Font("Segoe UI", 8.25F, FontStyle.Regular, GraphicsUnit.Point);
			propertyGrid1.Location = new Point(0, 0);
			propertyGrid1.Name = "propertyGrid1";
			propertyGrid1.Size = new Size(297, 457);
			propertyGrid1.TabIndex = 1;
			propertyGrid1.PropertyValueChanged += propertyGrid1_PropertyValueChanged;
			// 
			// splitContainer2
			// 
			splitContainer2.Dock = DockStyle.Fill;
			splitContainer2.FixedPanel = FixedPanel.Panel1;
			splitContainer2.Location = new Point(0, 0);
			splitContainer2.Name = "splitContainer2";
			splitContainer2.Orientation = Orientation.Horizontal;
			// 
			// splitContainer2.Panel1
			// 
			splitContainer2.Panel1.Controls.Add(button1);
			splitContainer2.Panel1.Controls.Add(comboBox1);
			// 
			// splitContainer2.Panel2
			// 
			splitContainer2.Panel2.Controls.Add(tabControl2);
			splitContainer2.Size = new Size(793, 503);
			splitContainer2.SplitterDistance = 42;
			splitContainer2.TabIndex = 0;
			// 
			// button1
			// 
			button1.Anchor = AnchorStyles.Right;
			button1.BackgroundImageLayout = ImageLayout.Zoom;
			button1.Image = Properties.Resources.button_get_debug;
			button1.Location = new Point(752, 5);
			button1.Name = "button1";
			button1.Size = new Size(32, 32);
			button1.TabIndex = 3;
			button1.UseVisualStyleBackColor = true;
			button1.Click += button1_Click;
			// 
			// comboBox1
			// 
			comboBox1.Anchor = AnchorStyles.Right;
			comboBox1.DropDownStyle = ComboBoxStyle.DropDownList;
			comboBox1.FormattingEnabled = true;
			comboBox1.Location = new Point(533, 9);
			comboBox1.Name = "comboBox1";
			comboBox1.Size = new Size(213, 23);
			comboBox1.TabIndex = 2;
			comboBox1.SelectedIndexChanged += comboBox1_SelectedIndexChanged;
			// 
			// tabControl2
			// 
			tabControl2.Controls.Add(tabPage5);
			tabControl2.Controls.Add(tabPage4);
			tabControl2.Controls.Add(tabPage1);
			tabControl2.Dock = DockStyle.Fill;
			tabControl2.Location = new Point(0, 0);
			tabControl2.Name = "tabControl2";
			tabControl2.SelectedIndex = 0;
			tabControl2.Size = new Size(793, 457);
			tabControl2.TabIndex = 1;
			// 
			// tabPage5
			// 
			tabPage5.Controls.Add(pictureBoxForHuman);
			tabPage5.Location = new Point(4, 24);
			tabPage5.Name = "tabPage5";
			tabPage5.Padding = new Padding(3);
			tabPage5.Size = new Size(785, 429);
			tabPage5.TabIndex = 1;
			tabPage5.Text = "Human";
			tabPage5.UseVisualStyleBackColor = true;
			// 
			// pictureBoxForHuman
			// 
			pictureBoxForHuman.BackColor = Color.Gainsboro;
			pictureBoxForHuman.Dock = DockStyle.Fill;
			pictureBoxForHuman.Location = new Point(3, 3);
			pictureBoxForHuman.Name = "pictureBoxForHuman";
			pictureBoxForHuman.Size = new Size(779, 423);
			pictureBoxForHuman.SizeMode = PictureBoxSizeMode.Zoom;
			pictureBoxForHuman.TabIndex = 2;
			pictureBoxForHuman.TabStop = false;
			// 
			// tabPage4
			// 
			tabPage4.Controls.Add(hexBox1);
			tabPage4.Location = new Point(4, 24);
			tabPage4.Name = "tabPage4";
			tabPage4.Padding = new Padding(3);
			tabPage4.Size = new Size(785, 429);
			tabPage4.TabIndex = 0;
			tabPage4.Text = "Hex";
			tabPage4.UseVisualStyleBackColor = true;
			// 
			// hexBox1
			// 
			hexBox1.ColumnInfoVisible = true;
			hexBox1.Dock = DockStyle.Fill;
			hexBox1.Font = new Font("Courier New", 8.25F, FontStyle.Regular, GraphicsUnit.Point);
			hexBox1.LineInfoVisible = true;
			hexBox1.Location = new Point(3, 3);
			hexBox1.Name = "hexBox1";
			hexBox1.ReadOnly = true;
			hexBox1.ShadowSelectionColor = Color.FromArgb(100, 60, 188, 255);
			hexBox1.Size = new Size(779, 423);
			hexBox1.StringViewVisible = true;
			hexBox1.TabIndex = 1;
			hexBox1.UseFixedBytesPerLine = true;
			hexBox1.VScrollBarVisible = true;
			// 
			// tabPage1
			// 
			tabPage1.Controls.Add(wavesControl1);
			tabPage1.Controls.Add(toolStrip2);
			tabPage1.Location = new Point(4, 24);
			tabPage1.Name = "tabPage1";
			tabPage1.Size = new Size(785, 429);
			tabPage1.TabIndex = 2;
			tabPage1.Text = "Waves";
			tabPage1.UseVisualStyleBackColor = true;
			// 
			// toolStrip2
			// 
			toolStrip2.Items.AddRange(new ToolStripItem[] { toolStripButton4, toolStripButton5, toolStripSeparator1, toolStripDropDownButton1, toolStripDropDownButton2, toolStripDropDownButton3 });
			toolStrip2.Location = new Point(0, 0);
			toolStrip2.Name = "toolStrip2";
			toolStrip2.Size = new Size(785, 25);
			toolStrip2.TabIndex = 0;
			toolStrip2.Text = "toolStrip2";
			// 
			// toolStripButton4
			// 
			toolStripButton4.Alignment = ToolStripItemAlignment.Right;
			toolStripButton4.DisplayStyle = ToolStripItemDisplayStyle.Text;
			toolStripButton4.Image = (Image)resources.GetObject("toolStripButton4.Image");
			toolStripButton4.ImageTransparentColor = Color.Magenta;
			toolStripButton4.Name = "toolStripButton4";
			toolStripButton4.Size = new Size(47, 22);
			toolStripButton4.Text = "Snatch";
			// 
			// toolStripButton5
			// 
			toolStripButton5.DisplayStyle = ToolStripItemDisplayStyle.Text;
			toolStripButton5.Image = (Image)resources.GetObject("toolStripButton5.Image");
			toolStripButton5.ImageTransparentColor = Color.Magenta;
			toolStripButton5.Name = "toolStripButton5";
			toolStripButton5.Size = new Size(38, 22);
			toolStripButton5.Text = "Clear";
			// 
			// toolStripSeparator1
			// 
			toolStripSeparator1.Name = "toolStripSeparator1";
			toolStripSeparator1.Size = new Size(6, 25);
			// 
			// toolStripDropDownButton1
			// 
			toolStripDropDownButton1.DisplayStyle = ToolStripItemDisplayStyle.Text;
			toolStripDropDownButton1.Image = (Image)resources.GetObject("toolStripDropDownButton1.Image");
			toolStripDropDownButton1.ImageTransparentColor = Color.Magenta;
			toolStripDropDownButton1.Name = "toolStripDropDownButton1";
			toolStripDropDownButton1.Size = new Size(45, 22);
			toolStripDropDownButton1.Text = "Core";
			// 
			// toolStripDropDownButton2
			// 
			toolStripDropDownButton2.DisplayStyle = ToolStripItemDisplayStyle.Text;
			toolStripDropDownButton2.Image = (Image)resources.GetObject("toolStripDropDownButton2.Image");
			toolStripDropDownButton2.ImageTransparentColor = Color.Magenta;
			toolStripDropDownButton2.Name = "toolStripDropDownButton2";
			toolStripDropDownButton2.Size = new Size(43, 22);
			toolStripDropDownButton2.Text = "APU";
			// 
			// toolStripDropDownButton3
			// 
			toolStripDropDownButton3.DisplayStyle = ToolStripItemDisplayStyle.Text;
			toolStripDropDownButton3.Image = (Image)resources.GetObject("toolStripDropDownButton3.Image");
			toolStripDropDownButton3.ImageTransparentColor = Color.Magenta;
			toolStripDropDownButton3.Name = "toolStripDropDownButton3";
			toolStripDropDownButton3.Size = new Size(42, 22);
			toolStripDropDownButton3.Text = "PPU";
			// 
			// wavesControl1
			// 
			wavesControl1.DottedColor = Color.Silver;
			wavesControl1.DottedOpacity = 95;
			wavesControl1.FillColor = Color.Black;
			wavesControl1.Font = new Font("Segoe UI", 6F, FontStyle.Regular, GraphicsUnit.Point);
			wavesControl1.GridColor = Color.Green;
			wavesControl1.GridOpacity = 95;
			wavesControl1.HighZColor = Color.Gold;
			wavesControl1.LabelsColor = Color.White;
			wavesControl1.Location = new Point(44, 50);
			wavesControl1.Name = "wavesControl1";
			wavesControl1.SelectionColor = Color.GhostWhite;
			wavesControl1.SignalColor = Color.SpringGreen;
			wavesControl1.Size = new Size(669, 336);
			wavesControl1.TabIndex = 1;
			wavesControl1.Text = "wavesControl1";
			wavesControl1.UndefinedColor = Color.Red;
			// 
			// FormDebugger
			// 
			AutoScaleDimensions = new SizeF(7F, 15F);
			AutoScaleMode = AutoScaleMode.Font;
			ClientSize = new Size(1094, 528);
			Controls.Add(splitContainer1);
			Controls.Add(toolStrip1);
			KeyPreview = true;
			Name = "FormDebugger";
			Text = "Debugger";
			Load += FormDebugger_Load;
			KeyDown += FormDebugger_KeyDown;
			toolStrip1.ResumeLayout(false);
			toolStrip1.PerformLayout();
			splitContainer1.Panel1.ResumeLayout(false);
			splitContainer1.Panel2.ResumeLayout(false);
			((System.ComponentModel.ISupportInitialize)splitContainer1).EndInit();
			splitContainer1.ResumeLayout(false);
			splitContainer3.Panel1.ResumeLayout(false);
			splitContainer3.Panel2.ResumeLayout(false);
			((System.ComponentModel.ISupportInitialize)splitContainer3).EndInit();
			splitContainer3.ResumeLayout(false);
			splitContainer2.Panel1.ResumeLayout(false);
			splitContainer2.Panel2.ResumeLayout(false);
			((System.ComponentModel.ISupportInitialize)splitContainer2).EndInit();
			splitContainer2.ResumeLayout(false);
			tabControl2.ResumeLayout(false);
			tabPage5.ResumeLayout(false);
			((System.ComponentModel.ISupportInitialize)pictureBoxForHuman).EndInit();
			tabPage4.ResumeLayout(false);
			tabPage1.ResumeLayout(false);
			tabPage1.PerformLayout();
			toolStrip2.ResumeLayout(false);
			toolStrip2.PerformLayout();
			ResumeLayout(false);
			PerformLayout();
		}

		#endregion

		private ToolStrip toolStrip1;
		private ToolStripButton toolStripButton1;
		private SplitContainer splitContainer1;
		private SplitContainer splitContainer3;
		private Button button2;
		private ComboBox comboBox2;
		private PropertyGrid propertyGrid1;
		private SplitContainer splitContainer2;
		private Button button1;
		private ComboBox comboBox1;
		private TabControl tabControl2;
		private TabPage tabPage5;
		private PictureBox pictureBoxForHuman;
		private TabPage tabPage4;
		private Be.Windows.Forms.HexBox hexBox1;
		private ToolStripButton toolStripButton2;
		private ToolStripButton toolStripButton3;
		private TabPage tabPage1;
		private ToolStrip toolStrip2;
		private ToolStripButton toolStripButton4;
		private ToolStripButton toolStripButton5;
		private ToolStripSeparator toolStripSeparator1;
		private ToolStripDropDownButton toolStripDropDownButton1;
		private ToolStripDropDownButton toolStripDropDownButton2;
		private ToolStripDropDownButton toolStripDropDownButton3;
		private WavesControl wavesControl1;
	}
}