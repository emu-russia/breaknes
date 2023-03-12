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
			this.toolStrip1 = new System.Windows.Forms.ToolStrip();
			this.toolStripButton1 = new System.Windows.Forms.ToolStripButton();
			this.toolStripButton2 = new System.Windows.Forms.ToolStripButton();
			this.toolStripButton3 = new System.Windows.Forms.ToolStripButton();
			this.splitContainer1 = new System.Windows.Forms.SplitContainer();
			this.splitContainer3 = new System.Windows.Forms.SplitContainer();
			this.button2 = new System.Windows.Forms.Button();
			this.comboBox2 = new System.Windows.Forms.ComboBox();
			this.propertyGrid1 = new System.Windows.Forms.PropertyGrid();
			this.splitContainer2 = new System.Windows.Forms.SplitContainer();
			this.button1 = new System.Windows.Forms.Button();
			this.comboBox1 = new System.Windows.Forms.ComboBox();
			this.tabControl2 = new System.Windows.Forms.TabControl();
			this.tabPage5 = new System.Windows.Forms.TabPage();
			this.pictureBoxForHuman = new System.Windows.Forms.PictureBox();
			this.tabPage4 = new System.Windows.Forms.TabPage();
			this.hexBox1 = new Be.Windows.Forms.HexBox();
			this.toolStrip1.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.splitContainer1)).BeginInit();
			this.splitContainer1.Panel1.SuspendLayout();
			this.splitContainer1.Panel2.SuspendLayout();
			this.splitContainer1.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.splitContainer3)).BeginInit();
			this.splitContainer3.Panel1.SuspendLayout();
			this.splitContainer3.Panel2.SuspendLayout();
			this.splitContainer3.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.splitContainer2)).BeginInit();
			this.splitContainer2.Panel1.SuspendLayout();
			this.splitContainer2.Panel2.SuspendLayout();
			this.splitContainer2.SuspendLayout();
			this.tabControl2.SuspendLayout();
			this.tabPage5.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.pictureBoxForHuman)).BeginInit();
			this.tabPage4.SuspendLayout();
			this.SuspendLayout();
			// 
			// toolStrip1
			// 
			this.toolStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripButton1,
            this.toolStripButton2,
            this.toolStripButton3});
			this.toolStrip1.Location = new System.Drawing.Point(0, 0);
			this.toolStrip1.Name = "toolStrip1";
			this.toolStrip1.Size = new System.Drawing.Size(882, 25);
			this.toolStrip1.TabIndex = 0;
			this.toolStrip1.Text = "toolStrip1";
			// 
			// toolStripButton1
			// 
			this.toolStripButton1.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Text;
			this.toolStripButton1.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButton1.Image")));
			this.toolStripButton1.ImageTransparentColor = System.Drawing.Color.Magenta;
			this.toolStripButton1.Name = "toolStripButton1";
			this.toolStripButton1.Size = new System.Drawing.Size(42, 22);
			this.toolStripButton1.Text = "Pause";
			this.toolStripButton1.Click += new System.EventHandler(this.toolStripButton1_Click);
			// 
			// toolStripButton2
			// 
			this.toolStripButton2.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Text;
			this.toolStripButton2.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButton2.Image")));
			this.toolStripButton2.ImageTransparentColor = System.Drawing.Color.Magenta;
			this.toolStripButton2.Name = "toolStripButton2";
			this.toolStripButton2.Size = new System.Drawing.Size(63, 22);
			this.toolStripButton2.Text = "Step (F11)";
			this.toolStripButton2.Click += new System.EventHandler(this.toolStripButton2_Click);
			// 
			// toolStripButton3
			// 
			this.toolStripButton3.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Text;
			this.toolStripButton3.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButton3.Image")));
			this.toolStripButton3.ImageTransparentColor = System.Drawing.Color.Magenta;
			this.toolStripButton3.Name = "toolStripButton3";
			this.toolStripButton3.Size = new System.Drawing.Size(55, 22);
			this.toolStripButton3.Text = "Run (F5)";
			this.toolStripButton3.Click += new System.EventHandler(this.toolStripButton3_Click);
			// 
			// splitContainer1
			// 
			this.splitContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.splitContainer1.Location = new System.Drawing.Point(0, 25);
			this.splitContainer1.Name = "splitContainer1";
			// 
			// splitContainer1.Panel1
			// 
			this.splitContainer1.Panel1.Controls.Add(this.splitContainer3);
			// 
			// splitContainer1.Panel2
			// 
			this.splitContainer1.Panel2.Controls.Add(this.splitContainer2);
			this.splitContainer1.Size = new System.Drawing.Size(882, 503);
			this.splitContainer1.SplitterDistance = 294;
			this.splitContainer1.TabIndex = 1;
			// 
			// splitContainer3
			// 
			this.splitContainer3.Dock = System.Windows.Forms.DockStyle.Fill;
			this.splitContainer3.FixedPanel = System.Windows.Forms.FixedPanel.Panel1;
			this.splitContainer3.Location = new System.Drawing.Point(0, 0);
			this.splitContainer3.Name = "splitContainer3";
			this.splitContainer3.Orientation = System.Windows.Forms.Orientation.Horizontal;
			// 
			// splitContainer3.Panel1
			// 
			this.splitContainer3.Panel1.Controls.Add(this.button2);
			this.splitContainer3.Panel1.Controls.Add(this.comboBox2);
			// 
			// splitContainer3.Panel2
			// 
			this.splitContainer3.Panel2.Controls.Add(this.propertyGrid1);
			this.splitContainer3.Size = new System.Drawing.Size(294, 503);
			this.splitContainer3.SplitterDistance = 42;
			this.splitContainer3.TabIndex = 1;
			// 
			// button2
			// 
			this.button2.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Zoom;
			this.button2.Image = global::Breaknes.Properties.Resources.button_get_debug;
			this.button2.Location = new System.Drawing.Point(248, 5);
			this.button2.Name = "button2";
			this.button2.Size = new System.Drawing.Size(32, 32);
			this.button2.TabIndex = 2;
			this.button2.UseVisualStyleBackColor = true;
			this.button2.Click += new System.EventHandler(this.button2_Click);
			// 
			// comboBox2
			// 
			this.comboBox2.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.comboBox2.FormattingEnabled = true;
			this.comboBox2.Items.AddRange(new object[] {
            "Core",
            "CoreRegs",
            "APU",
            "APURegs",
            "PPU",
            "PPURegs",
            "Board",
            "Cartridge"});
			this.comboBox2.Location = new System.Drawing.Point(8, 9);
			this.comboBox2.Name = "comboBox2";
			this.comboBox2.Size = new System.Drawing.Size(234, 23);
			this.comboBox2.TabIndex = 0;
			this.comboBox2.SelectedIndexChanged += new System.EventHandler(this.comboBox2_SelectedIndexChanged);
			// 
			// propertyGrid1
			// 
			this.propertyGrid1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.propertyGrid1.Font = new System.Drawing.Font("Segoe UI", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
			this.propertyGrid1.Location = new System.Drawing.Point(0, 0);
			this.propertyGrid1.Name = "propertyGrid1";
			this.propertyGrid1.Size = new System.Drawing.Size(294, 457);
			this.propertyGrid1.TabIndex = 1;
			this.propertyGrid1.PropertyValueChanged += new System.Windows.Forms.PropertyValueChangedEventHandler(this.propertyGrid1_PropertyValueChanged);
			// 
			// splitContainer2
			// 
			this.splitContainer2.Dock = System.Windows.Forms.DockStyle.Fill;
			this.splitContainer2.FixedPanel = System.Windows.Forms.FixedPanel.Panel1;
			this.splitContainer2.Location = new System.Drawing.Point(0, 0);
			this.splitContainer2.Name = "splitContainer2";
			this.splitContainer2.Orientation = System.Windows.Forms.Orientation.Horizontal;
			// 
			// splitContainer2.Panel1
			// 
			this.splitContainer2.Panel1.Controls.Add(this.button1);
			this.splitContainer2.Panel1.Controls.Add(this.comboBox1);
			// 
			// splitContainer2.Panel2
			// 
			this.splitContainer2.Panel2.Controls.Add(this.tabControl2);
			this.splitContainer2.Size = new System.Drawing.Size(584, 503);
			this.splitContainer2.SplitterDistance = 42;
			this.splitContainer2.TabIndex = 0;
			// 
			// button1
			// 
			this.button1.Anchor = System.Windows.Forms.AnchorStyles.Right;
			this.button1.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Zoom;
			this.button1.Image = global::Breaknes.Properties.Resources.button_get_debug;
			this.button1.Location = new System.Drawing.Point(543, 5);
			this.button1.Name = "button1";
			this.button1.Size = new System.Drawing.Size(32, 32);
			this.button1.TabIndex = 3;
			this.button1.UseVisualStyleBackColor = true;
			this.button1.Click += new System.EventHandler(this.button1_Click);
			// 
			// comboBox1
			// 
			this.comboBox1.Anchor = System.Windows.Forms.AnchorStyles.Right;
			this.comboBox1.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.comboBox1.FormattingEnabled = true;
			this.comboBox1.Location = new System.Drawing.Point(324, 9);
			this.comboBox1.Name = "comboBox1";
			this.comboBox1.Size = new System.Drawing.Size(213, 23);
			this.comboBox1.TabIndex = 2;
			this.comboBox1.SelectedIndexChanged += new System.EventHandler(this.comboBox1_SelectedIndexChanged);
			// 
			// tabControl2
			// 
			this.tabControl2.Controls.Add(this.tabPage5);
			this.tabControl2.Controls.Add(this.tabPage4);
			this.tabControl2.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tabControl2.Location = new System.Drawing.Point(0, 0);
			this.tabControl2.Name = "tabControl2";
			this.tabControl2.SelectedIndex = 0;
			this.tabControl2.Size = new System.Drawing.Size(584, 457);
			this.tabControl2.TabIndex = 1;
			// 
			// tabPage5
			// 
			this.tabPage5.Controls.Add(this.pictureBoxForHuman);
			this.tabPage5.Location = new System.Drawing.Point(4, 24);
			this.tabPage5.Name = "tabPage5";
			this.tabPage5.Padding = new System.Windows.Forms.Padding(3);
			this.tabPage5.Size = new System.Drawing.Size(576, 429);
			this.tabPage5.TabIndex = 1;
			this.tabPage5.Text = "Human";
			this.tabPage5.UseVisualStyleBackColor = true;
			// 
			// pictureBoxForHuman
			// 
			this.pictureBoxForHuman.BackColor = System.Drawing.Color.Gainsboro;
			this.pictureBoxForHuman.Dock = System.Windows.Forms.DockStyle.Fill;
			this.pictureBoxForHuman.Location = new System.Drawing.Point(3, 3);
			this.pictureBoxForHuman.Name = "pictureBoxForHuman";
			this.pictureBoxForHuman.Size = new System.Drawing.Size(570, 423);
			this.pictureBoxForHuman.SizeMode = System.Windows.Forms.PictureBoxSizeMode.Zoom;
			this.pictureBoxForHuman.TabIndex = 2;
			this.pictureBoxForHuman.TabStop = false;
			// 
			// tabPage4
			// 
			this.tabPage4.Controls.Add(this.hexBox1);
			this.tabPage4.Location = new System.Drawing.Point(4, 24);
			this.tabPage4.Name = "tabPage4";
			this.tabPage4.Padding = new System.Windows.Forms.Padding(3);
			this.tabPage4.Size = new System.Drawing.Size(576, 429);
			this.tabPage4.TabIndex = 0;
			this.tabPage4.Text = "Hex";
			this.tabPage4.UseVisualStyleBackColor = true;
			// 
			// hexBox1
			// 
			this.hexBox1.ColumnInfoVisible = true;
			this.hexBox1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.hexBox1.Font = new System.Drawing.Font("Courier New", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
			this.hexBox1.LineInfoVisible = true;
			this.hexBox1.Location = new System.Drawing.Point(3, 3);
			this.hexBox1.Name = "hexBox1";
			this.hexBox1.ReadOnly = true;
			this.hexBox1.ShadowSelectionColor = System.Drawing.Color.FromArgb(((int)(((byte)(100)))), ((int)(((byte)(60)))), ((int)(((byte)(188)))), ((int)(((byte)(255)))));
			this.hexBox1.Size = new System.Drawing.Size(570, 423);
			this.hexBox1.StringViewVisible = true;
			this.hexBox1.TabIndex = 1;
			this.hexBox1.UseFixedBytesPerLine = true;
			this.hexBox1.VScrollBarVisible = true;
			// 
			// FormDebugger
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(882, 528);
			this.Controls.Add(this.splitContainer1);
			this.Controls.Add(this.toolStrip1);
			this.KeyPreview = true;
			this.Name = "FormDebugger";
			this.Text = "Debugger";
			this.Load += new System.EventHandler(this.FormDebugger_Load);
			this.KeyDown += new System.Windows.Forms.KeyEventHandler(this.FormDebugger_KeyDown);
			this.toolStrip1.ResumeLayout(false);
			this.toolStrip1.PerformLayout();
			this.splitContainer1.Panel1.ResumeLayout(false);
			this.splitContainer1.Panel2.ResumeLayout(false);
			((System.ComponentModel.ISupportInitialize)(this.splitContainer1)).EndInit();
			this.splitContainer1.ResumeLayout(false);
			this.splitContainer3.Panel1.ResumeLayout(false);
			this.splitContainer3.Panel2.ResumeLayout(false);
			((System.ComponentModel.ISupportInitialize)(this.splitContainer3)).EndInit();
			this.splitContainer3.ResumeLayout(false);
			this.splitContainer2.Panel1.ResumeLayout(false);
			this.splitContainer2.Panel2.ResumeLayout(false);
			((System.ComponentModel.ISupportInitialize)(this.splitContainer2)).EndInit();
			this.splitContainer2.ResumeLayout(false);
			this.tabControl2.ResumeLayout(false);
			this.tabPage5.ResumeLayout(false);
			((System.ComponentModel.ISupportInitialize)(this.pictureBoxForHuman)).EndInit();
			this.tabPage4.ResumeLayout(false);
			this.ResumeLayout(false);
			this.PerformLayout();

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
	}
}