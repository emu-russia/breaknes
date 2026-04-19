
namespace BreaksDebug
{
	partial class FormMain
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
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(FormMain));
			menuStrip1 = new MenuStrip();
			fileToolStripMenuItem = new ToolStripMenuItem();
			loadAssemblySourceToolStripMenuItem = new ToolStripMenuItem();
			toolStripSeparator4 = new ToolStripSeparator();
			loadMemoryDumpToolStripMenuItem = new ToolStripMenuItem();
			saveTheMemoryDumpToolStripMenuItem = new ToolStripMenuItem();
			toolStripSeparator1 = new ToolStripSeparator();
			exitToolStripMenuItem = new ToolStripMenuItem();
			helpToolStripMenuItem = new ToolStripMenuItem();
			sendFeedbackToolStripMenuItem = new ToolStripMenuItem();
			checkForUpdatesToolStripMenuItem = new ToolStripMenuItem();
			toolStripSeparator5 = new ToolStripSeparator();
			aboutToolStripMenuItem = new ToolStripMenuItem();
			toolStrip1 = new ToolStrip();
			toolStripButton1 = new ToolStripButton();
			toolStripSeparator2 = new ToolStripSeparator();
			toolStripButton4 = new ToolStripButton();
			toolStripButton5 = new ToolStripButton();
			toolStripButton6 = new ToolStripButton();
			toolStripSeparator3 = new ToolStripSeparator();
			toolStripButton3 = new ToolStripButton();
			toolStripButton2 = new ToolStripButton();
			statusStrip1 = new StatusStrip();
			toolStripStatusLabel1 = new ToolStripStatusLabel();
			openFileDialog1 = new OpenFileDialog();
			openFileDialog2 = new OpenFileDialog();
			saveFileDialog1 = new SaveFileDialog();
			tabControl2 = new TabControl();
			tabPage3 = new TabPage();
			tableLayoutPanel1 = new TableLayoutPanel();
			splitContainer4 = new SplitContainer();
			label1 = new Label();
			label9 = new Label();
			propertyGrid1 = new PropertyGrid();
			propertyGrid2 = new PropertyGrid();
			propertyGrid3 = new PropertyGrid();
			propertyGrid4 = new PropertyGrid();
			propertyGrid5 = new PropertyGrid();
			label5 = new Label();
			label6 = new Label();
			label7 = new Label();
			label8 = new Label();
			splitContainer2 = new SplitContainer();
			label2 = new Label();
			label3 = new Label();
			label4 = new Label();
			tabPage4 = new TabPage();
			splitContainer3 = new SplitContainer();
			richTextBox1 = new RichTextBox();
			button2 = new Button();
			tabPage5 = new TabPage();
			wavesControl1 = new WavesControl();
			toolStrip2 = new ToolStrip();
			toolStripButton7 = new ToolStripButton();
			toolStripButton8 = new ToolStripButton();
			toolStripSeparator6 = new ToolStripSeparator();
			toolStripButton9 = new ToolStripButton();
			toolStripButton10 = new ToolStripButton();
			toolStripButton11 = new ToolStripButton();
			toolStripButton12 = new ToolStripButton();
			toolStripButton13 = new ToolStripButton();
			tabPage2 = new TabPage();
			dataPathView21 = new DataPathView();
			tabPage6 = new TabPage();
			hexBox1 = new Be.Windows.Forms.HexBox();
			menuStrip1.SuspendLayout();
			toolStrip1.SuspendLayout();
			statusStrip1.SuspendLayout();
			tabControl2.SuspendLayout();
			tabPage3.SuspendLayout();
			tableLayoutPanel1.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)splitContainer4).BeginInit();
			splitContainer4.Panel1.SuspendLayout();
			splitContainer4.Panel2.SuspendLayout();
			splitContainer4.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)splitContainer2).BeginInit();
			splitContainer2.Panel1.SuspendLayout();
			splitContainer2.Panel2.SuspendLayout();
			splitContainer2.SuspendLayout();
			tabPage4.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)splitContainer3).BeginInit();
			splitContainer3.Panel1.SuspendLayout();
			splitContainer3.Panel2.SuspendLayout();
			splitContainer3.SuspendLayout();
			tabPage5.SuspendLayout();
			toolStrip2.SuspendLayout();
			tabPage2.SuspendLayout();
			tabPage6.SuspendLayout();
			SuspendLayout();
			// 
			// menuStrip1
			// 
			menuStrip1.ImageScalingSize = new Size(20, 20);
			menuStrip1.Items.AddRange(new ToolStripItem[] { fileToolStripMenuItem, helpToolStripMenuItem });
			menuStrip1.Location = new Point(0, 0);
			menuStrip1.Name = "menuStrip1";
			menuStrip1.Padding = new Padding(8, 3, 0, 3);
			menuStrip1.Size = new Size(1825, 30);
			menuStrip1.TabIndex = 0;
			menuStrip1.Text = "menuStrip1";
			// 
			// fileToolStripMenuItem
			// 
			fileToolStripMenuItem.DropDownItems.AddRange(new ToolStripItem[] { loadAssemblySourceToolStripMenuItem, toolStripSeparator4, loadMemoryDumpToolStripMenuItem, saveTheMemoryDumpToolStripMenuItem, toolStripSeparator1, exitToolStripMenuItem });
			fileToolStripMenuItem.Name = "fileToolStripMenuItem";
			fileToolStripMenuItem.Size = new Size(46, 24);
			fileToolStripMenuItem.Text = "File";
			// 
			// loadAssemblySourceToolStripMenuItem
			// 
			loadAssemblySourceToolStripMenuItem.Name = "loadAssemblySourceToolStripMenuItem";
			loadAssemblySourceToolStripMenuItem.Size = new Size(250, 26);
			loadAssemblySourceToolStripMenuItem.Text = "Load assembly source";
			loadAssemblySourceToolStripMenuItem.Click += loadAssemblySourceToolStripMenuItem_Click;
			// 
			// toolStripSeparator4
			// 
			toolStripSeparator4.Name = "toolStripSeparator4";
			toolStripSeparator4.Size = new Size(247, 6);
			// 
			// loadMemoryDumpToolStripMenuItem
			// 
			loadMemoryDumpToolStripMenuItem.Name = "loadMemoryDumpToolStripMenuItem";
			loadMemoryDumpToolStripMenuItem.Size = new Size(250, 26);
			loadMemoryDumpToolStripMenuItem.Text = "Load memory dump";
			loadMemoryDumpToolStripMenuItem.Click += loadMemoryDumpToolStripMenuItem_Click;
			// 
			// saveTheMemoryDumpToolStripMenuItem
			// 
			saveTheMemoryDumpToolStripMenuItem.Name = "saveTheMemoryDumpToolStripMenuItem";
			saveTheMemoryDumpToolStripMenuItem.Size = new Size(250, 26);
			saveTheMemoryDumpToolStripMenuItem.Text = "Save the memory dump";
			saveTheMemoryDumpToolStripMenuItem.Click += saveTheMemoryDumpToolStripMenuItem_Click;
			// 
			// toolStripSeparator1
			// 
			toolStripSeparator1.Name = "toolStripSeparator1";
			toolStripSeparator1.Size = new Size(247, 6);
			// 
			// exitToolStripMenuItem
			// 
			exitToolStripMenuItem.Name = "exitToolStripMenuItem";
			exitToolStripMenuItem.Size = new Size(250, 26);
			exitToolStripMenuItem.Text = "Exit";
			exitToolStripMenuItem.Click += exitToolStripMenuItem_Click;
			// 
			// helpToolStripMenuItem
			// 
			helpToolStripMenuItem.DropDownItems.AddRange(new ToolStripItem[] { sendFeedbackToolStripMenuItem, checkForUpdatesToolStripMenuItem, toolStripSeparator5, aboutToolStripMenuItem });
			helpToolStripMenuItem.Name = "helpToolStripMenuItem";
			helpToolStripMenuItem.Size = new Size(55, 24);
			helpToolStripMenuItem.Text = "Help";
			// 
			// sendFeedbackToolStripMenuItem
			// 
			sendFeedbackToolStripMenuItem.Name = "sendFeedbackToolStripMenuItem";
			sendFeedbackToolStripMenuItem.Size = new Size(211, 26);
			sendFeedbackToolStripMenuItem.Text = "Send feedback";
			sendFeedbackToolStripMenuItem.Click += sendFeedbackToolStripMenuItem_Click;
			// 
			// checkForUpdatesToolStripMenuItem
			// 
			checkForUpdatesToolStripMenuItem.Name = "checkForUpdatesToolStripMenuItem";
			checkForUpdatesToolStripMenuItem.Size = new Size(211, 26);
			checkForUpdatesToolStripMenuItem.Text = "Check for updates";
			checkForUpdatesToolStripMenuItem.Click += checkForUpdatesToolStripMenuItem_Click;
			// 
			// toolStripSeparator5
			// 
			toolStripSeparator5.Name = "toolStripSeparator5";
			toolStripSeparator5.Size = new Size(208, 6);
			// 
			// aboutToolStripMenuItem
			// 
			aboutToolStripMenuItem.Name = "aboutToolStripMenuItem";
			aboutToolStripMenuItem.Size = new Size(211, 26);
			aboutToolStripMenuItem.Text = "About";
			aboutToolStripMenuItem.Click += aboutToolStripMenuItem_Click;
			// 
			// toolStrip1
			// 
			toolStrip1.ImageScalingSize = new Size(20, 20);
			toolStrip1.Items.AddRange(new ToolStripItem[] { toolStripButton1, toolStripSeparator2, toolStripButton4, toolStripButton5, toolStripButton6, toolStripSeparator3, toolStripButton3, toolStripButton2 });
			toolStrip1.Location = new Point(0, 30);
			toolStrip1.Name = "toolStrip1";
			toolStrip1.Size = new Size(1825, 39);
			toolStrip1.TabIndex = 1;
			toolStrip1.Text = "toolStrip1";
			// 
			// toolStripButton1
			// 
			toolStripButton1.Image = (Image)resources.GetObject("toolStripButton1.Image");
			toolStripButton1.ImageScaling = ToolStripItemImageScaling.None;
			toolStripButton1.ImageTransparentColor = Color.Magenta;
			toolStripButton1.Name = "toolStripButton1";
			toolStripButton1.Size = new Size(116, 36);
			toolStripButton1.Text = "Step (F11)";
			toolStripButton1.Click += toolStripButton1_Click;
			// 
			// toolStripSeparator2
			// 
			toolStripSeparator2.Name = "toolStripSeparator2";
			toolStripSeparator2.Size = new Size(6, 39);
			// 
			// toolStripButton4
			// 
			toolStripButton4.CheckOnClick = true;
			toolStripButton4.DisplayStyle = ToolStripItemDisplayStyle.Text;
			toolStripButton4.Image = (Image)resources.GetObject("toolStripButton4.Image");
			toolStripButton4.ImageTransparentColor = Color.Magenta;
			toolStripButton4.Name = "toolStripButton4";
			toolStripButton4.Size = new Size(47, 36);
			toolStripButton4.Text = "/NMI";
			toolStripButton4.CheckedChanged += toolStripButton4_CheckedChanged;
			// 
			// toolStripButton5
			// 
			toolStripButton5.CheckOnClick = true;
			toolStripButton5.DisplayStyle = ToolStripItemDisplayStyle.Text;
			toolStripButton5.Image = (Image)resources.GetObject("toolStripButton5.Image");
			toolStripButton5.ImageTransparentColor = Color.Magenta;
			toolStripButton5.Name = "toolStripButton5";
			toolStripButton5.Size = new Size(43, 36);
			toolStripButton5.Text = "/IRQ";
			toolStripButton5.CheckedChanged += toolStripButton5_CheckedChanged;
			// 
			// toolStripButton6
			// 
			toolStripButton6.CheckOnClick = true;
			toolStripButton6.DisplayStyle = ToolStripItemDisplayStyle.Text;
			toolStripButton6.Image = (Image)resources.GetObject("toolStripButton6.Image");
			toolStripButton6.ImageTransparentColor = Color.Magenta;
			toolStripButton6.Name = "toolStripButton6";
			toolStripButton6.Size = new Size(44, 36);
			toolStripButton6.Text = "/RES";
			toolStripButton6.CheckedChanged += toolStripButton6_CheckedChanged;
			// 
			// toolStripSeparator3
			// 
			toolStripSeparator3.Name = "toolStripSeparator3";
			toolStripSeparator3.Size = new Size(6, 39);
			// 
			// toolStripButton3
			// 
			toolStripButton3.CheckOnClick = true;
			toolStripButton3.DisplayStyle = ToolStripItemDisplayStyle.Text;
			toolStripButton3.Image = (Image)resources.GetObject("toolStripButton3.Image");
			toolStripButton3.ImageTransparentColor = Color.Magenta;
			toolStripButton3.Name = "toolStripButton3";
			toolStripButton3.Size = new Size(32, 36);
			toolStripButton3.Text = "SO";
			toolStripButton3.CheckedChanged += toolStripButton3_CheckedChanged;
			// 
			// toolStripButton2
			// 
			toolStripButton2.CheckOnClick = true;
			toolStripButton2.DisplayStyle = ToolStripItemDisplayStyle.Text;
			toolStripButton2.Image = (Image)resources.GetObject("toolStripButton2.Image");
			toolStripButton2.ImageTransparentColor = Color.Magenta;
			toolStripButton2.Name = "toolStripButton2";
			toolStripButton2.Size = new Size(41, 36);
			toolStripButton2.Text = "RDY";
			toolStripButton2.CheckedChanged += toolStripButton2_CheckedChanged;
			// 
			// statusStrip1
			// 
			statusStrip1.ImageScalingSize = new Size(20, 20);
			statusStrip1.Items.AddRange(new ToolStripItem[] { toolStripStatusLabel1 });
			statusStrip1.Location = new Point(0, 902);
			statusStrip1.Name = "statusStrip1";
			statusStrip1.Padding = new Padding(1, 0, 18, 0);
			statusStrip1.Size = new Size(1825, 26);
			statusStrip1.TabIndex = 2;
			statusStrip1.Text = "statusStrip1";
			// 
			// toolStripStatusLabel1
			// 
			toolStripStatusLabel1.Name = "toolStripStatusLabel1";
			toolStripStatusLabel1.Size = new Size(151, 20);
			toolStripStatusLabel1.Text = "toolStripStatusLabel1";
			// 
			// openFileDialog1
			// 
			openFileDialog1.DefaultExt = "asm";
			openFileDialog1.Filter = "ASM files|*.asm;*.nas|All files|*.*";
			// 
			// openFileDialog2
			// 
			openFileDialog2.DefaultExt = "bin";
			openFileDialog2.Filter = "Binary files|*.bin|All files|*.*";
			// 
			// saveFileDialog1
			// 
			saveFileDialog1.DefaultExt = "bin";
			saveFileDialog1.Filter = "Binary files|*.bin|All files|*.*";
			// 
			// tabControl2
			// 
			tabControl2.Controls.Add(tabPage3);
			tabControl2.Controls.Add(tabPage4);
			tabControl2.Controls.Add(tabPage5);
			tabControl2.Controls.Add(tabPage2);
			tabControl2.Controls.Add(tabPage6);
			tabControl2.Dock = DockStyle.Fill;
			tabControl2.Location = new Point(0, 69);
			tabControl2.Margin = new Padding(5, 4, 5, 4);
			tabControl2.Name = "tabControl2";
			tabControl2.SelectedIndex = 0;
			tabControl2.Size = new Size(1825, 833);
			tabControl2.TabIndex = 3;
			// 
			// tabPage3
			// 
			tabPage3.Controls.Add(tableLayoutPanel1);
			tabPage3.Location = new Point(4, 29);
			tabPage3.Margin = new Padding(5, 4, 5, 4);
			tabPage3.Name = "tabPage3";
			tabPage3.Padding = new Padding(5, 4, 5, 4);
			tabPage3.Size = new Size(1817, 800);
			tabPage3.TabIndex = 0;
			tabPage3.Text = "6502 Core";
			tabPage3.UseVisualStyleBackColor = true;
			// 
			// tableLayoutPanel1
			// 
			tableLayoutPanel1.ColumnCount = 5;
			tableLayoutPanel1.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 20F));
			tableLayoutPanel1.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 20F));
			tableLayoutPanel1.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 20F));
			tableLayoutPanel1.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 20F));
			tableLayoutPanel1.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 20F));
			tableLayoutPanel1.Controls.Add(splitContainer4, 1, 0);
			tableLayoutPanel1.Controls.Add(propertyGrid1, 0, 2);
			tableLayoutPanel1.Controls.Add(propertyGrid2, 1, 2);
			tableLayoutPanel1.Controls.Add(propertyGrid3, 2, 2);
			tableLayoutPanel1.Controls.Add(propertyGrid4, 3, 2);
			tableLayoutPanel1.Controls.Add(propertyGrid5, 4, 2);
			tableLayoutPanel1.Controls.Add(label5, 1, 1);
			tableLayoutPanel1.Controls.Add(label6, 2, 1);
			tableLayoutPanel1.Controls.Add(label7, 3, 1);
			tableLayoutPanel1.Controls.Add(label8, 4, 1);
			tableLayoutPanel1.Controls.Add(splitContainer2, 0, 0);
			tableLayoutPanel1.Controls.Add(label4, 0, 1);
			tableLayoutPanel1.Dock = DockStyle.Fill;
			tableLayoutPanel1.Location = new Point(5, 4);
			tableLayoutPanel1.Margin = new Padding(5, 4, 5, 4);
			tableLayoutPanel1.Name = "tableLayoutPanel1";
			tableLayoutPanel1.RowCount = 3;
			tableLayoutPanel1.RowStyles.Add(new RowStyle(SizeType.Absolute, 31F));
			tableLayoutPanel1.RowStyles.Add(new RowStyle(SizeType.Absolute, 31F));
			tableLayoutPanel1.RowStyles.Add(new RowStyle());
			tableLayoutPanel1.RowStyles.Add(new RowStyle(SizeType.Absolute, 31F));
			tableLayoutPanel1.Size = new Size(1807, 792);
			tableLayoutPanel1.TabIndex = 12;
			// 
			// splitContainer4
			// 
			splitContainer4.Dock = DockStyle.Fill;
			splitContainer4.FixedPanel = FixedPanel.Panel1;
			splitContainer4.Location = new Point(366, 4);
			splitContainer4.Margin = new Padding(5, 4, 5, 4);
			splitContainer4.Name = "splitContainer4";
			// 
			// splitContainer4.Panel1
			// 
			splitContainer4.Panel1.Controls.Add(label1);
			// 
			// splitContainer4.Panel2
			// 
			splitContainer4.Panel2.Controls.Add(label9);
			splitContainer4.Size = new Size(351, 23);
			splitContainer4.SplitterDistance = 59;
			splitContainer4.SplitterWidth = 6;
			splitContainer4.TabIndex = 16;
			// 
			// label1
			// 
			label1.AutoSize = true;
			label1.Location = new Point(0, 0);
			label1.Margin = new Padding(5, 0, 5, 0);
			label1.Name = "label1";
			label1.Size = new Size(65, 20);
			label1.TabIndex = 0;
			label1.Text = "Opcode:";
			// 
			// label9
			// 
			label9.AutoSize = true;
			label9.Font = new Font("Microsoft Sans Serif", 8.25F, FontStyle.Bold);
			label9.Location = new Point(5, 0);
			label9.Margin = new Padding(5, 0, 5, 0);
			label9.Name = "label9";
			label9.Size = new Size(35, 17);
			label9.TabIndex = 1;
			label9.Text = "???";
			// 
			// propertyGrid1
			// 
			propertyGrid1.BackColor = SystemColors.Control;
			propertyGrid1.Dock = DockStyle.Fill;
			propertyGrid1.Font = new Font("Microsoft Sans Serif", 6.75F);
			propertyGrid1.HelpVisible = false;
			propertyGrid1.Location = new Point(5, 66);
			propertyGrid1.Margin = new Padding(5, 4, 5, 4);
			propertyGrid1.Name = "propertyGrid1";
			propertyGrid1.Size = new Size(351, 722);
			propertyGrid1.TabIndex = 13;
			// 
			// propertyGrid2
			// 
			propertyGrid2.BackColor = SystemColors.Control;
			propertyGrid2.Dock = DockStyle.Fill;
			propertyGrid2.Font = new Font("Microsoft Sans Serif", 6.75F);
			propertyGrid2.HelpVisible = false;
			propertyGrid2.Location = new Point(366, 66);
			propertyGrid2.Margin = new Padding(5, 4, 5, 4);
			propertyGrid2.Name = "propertyGrid2";
			propertyGrid2.Size = new Size(351, 722);
			propertyGrid2.TabIndex = 4;
			// 
			// propertyGrid3
			// 
			propertyGrid3.BackColor = SystemColors.Control;
			propertyGrid3.Dock = DockStyle.Fill;
			propertyGrid3.Font = new Font("Microsoft Sans Serif", 6.75F);
			propertyGrid3.HelpVisible = false;
			propertyGrid3.Location = new Point(727, 66);
			propertyGrid3.Margin = new Padding(5, 4, 5, 4);
			propertyGrid3.Name = "propertyGrid3";
			propertyGrid3.Size = new Size(351, 722);
			propertyGrid3.TabIndex = 6;
			// 
			// propertyGrid4
			// 
			propertyGrid4.BackColor = SystemColors.Control;
			propertyGrid4.Dock = DockStyle.Fill;
			propertyGrid4.Font = new Font("Microsoft Sans Serif", 6.75F);
			propertyGrid4.HelpVisible = false;
			propertyGrid4.Location = new Point(1088, 66);
			propertyGrid4.Margin = new Padding(5, 4, 5, 4);
			propertyGrid4.Name = "propertyGrid4";
			propertyGrid4.Size = new Size(351, 722);
			propertyGrid4.TabIndex = 8;
			// 
			// propertyGrid5
			// 
			propertyGrid5.BackColor = SystemColors.Control;
			propertyGrid5.Dock = DockStyle.Fill;
			propertyGrid5.Font = new Font("Microsoft Sans Serif", 6.75F);
			propertyGrid5.HelpVisible = false;
			propertyGrid5.Location = new Point(1449, 66);
			propertyGrid5.Margin = new Padding(5, 4, 5, 4);
			propertyGrid5.Name = "propertyGrid5";
			propertyGrid5.Size = new Size(353, 722);
			propertyGrid5.TabIndex = 10;
			// 
			// label5
			// 
			label5.AutoSize = true;
			label5.Location = new Point(366, 31);
			label5.Margin = new Padding(5, 0, 5, 0);
			label5.Name = "label5";
			label5.Size = new Size(101, 20);
			label5.TabIndex = 5;
			label5.Text = "Regs && Buses:";
			// 
			// label6
			// 
			label6.AutoSize = true;
			label6.Location = new Point(727, 31);
			label6.Margin = new Padding(5, 0, 5, 0);
			label6.Name = "label6";
			label6.Size = new Size(103, 20);
			label6.TabIndex = 7;
			label6.Text = "Core Internals:";
			// 
			// label7
			// 
			label7.AutoSize = true;
			label7.Location = new Point(1088, 31);
			label7.Margin = new Padding(5, 0, 5, 0);
			label7.Name = "label7";
			label7.Size = new Size(69, 20);
			label7.TabIndex = 9;
			label7.Text = "Decoder:";
			// 
			// label8
			// 
			label8.AutoSize = true;
			label8.Location = new Point(1449, 31);
			label8.Margin = new Padding(5, 0, 5, 0);
			label8.Name = "label8";
			label8.Size = new Size(138, 20);
			label8.TabIndex = 11;
			label8.Text = "Control commands:";
			// 
			// splitContainer2
			// 
			splitContainer2.Dock = DockStyle.Fill;
			splitContainer2.FixedPanel = FixedPanel.Panel1;
			splitContainer2.Location = new Point(5, 4);
			splitContainer2.Margin = new Padding(5, 4, 5, 4);
			splitContainer2.Name = "splitContainer2";
			// 
			// splitContainer2.Panel1
			// 
			splitContainer2.Panel1.Controls.Add(label2);
			// 
			// splitContainer2.Panel2
			// 
			splitContainer2.Panel2.Controls.Add(label3);
			splitContainer2.Size = new Size(351, 23);
			splitContainer2.SplitterDistance = 47;
			splitContainer2.SplitterWidth = 6;
			splitContainer2.TabIndex = 15;
			// 
			// label2
			// 
			label2.AutoSize = true;
			label2.Location = new Point(0, 0);
			label2.Margin = new Padding(5, 0, 5, 0);
			label2.Name = "label2";
			label2.Size = new Size(46, 20);
			label2.TabIndex = 0;
			label2.Text = "State:";
			// 
			// label3
			// 
			label3.AutoSize = true;
			label3.Font = new Font("Microsoft Sans Serif", 8.25F, FontStyle.Bold);
			label3.Location = new Point(5, 0);
			label3.Margin = new Padding(5, 0, 5, 0);
			label3.Name = "label3";
			label3.Size = new Size(134, 17);
			label3.TabIndex = 1;
			label3.Text = "Not yet simulated";
			// 
			// label4
			// 
			label4.AutoSize = true;
			label4.Location = new Point(5, 31);
			label4.Margin = new Padding(5, 0, 5, 0);
			label4.Name = "label4";
			label4.Size = new Size(42, 20);
			label4.TabIndex = 14;
			label4.Text = "Pads:";
			// 
			// tabPage4
			// 
			tabPage4.Controls.Add(splitContainer3);
			tabPage4.Location = new Point(4, 29);
			tabPage4.Margin = new Padding(5, 4, 5, 4);
			tabPage4.Name = "tabPage4";
			tabPage4.Padding = new Padding(5, 4, 5, 4);
			tabPage4.Size = new Size(1817, 800);
			tabPage4.TabIndex = 1;
			tabPage4.Text = "Assembler";
			tabPage4.UseVisualStyleBackColor = true;
			// 
			// splitContainer3
			// 
			splitContainer3.Dock = DockStyle.Fill;
			splitContainer3.FixedPanel = FixedPanel.Panel2;
			splitContainer3.Location = new Point(5, 4);
			splitContainer3.Margin = new Padding(5, 4, 5, 4);
			splitContainer3.Name = "splitContainer3";
			splitContainer3.Orientation = Orientation.Horizontal;
			// 
			// splitContainer3.Panel1
			// 
			splitContainer3.Panel1.Controls.Add(richTextBox1);
			// 
			// splitContainer3.Panel2
			// 
			splitContainer3.Panel2.Controls.Add(button2);
			splitContainer3.Size = new Size(1807, 792);
			splitContainer3.SplitterDistance = 701;
			splitContainer3.SplitterWidth = 7;
			splitContainer3.TabIndex = 0;
			// 
			// richTextBox1
			// 
			richTextBox1.AcceptsTab = true;
			richTextBox1.Dock = DockStyle.Fill;
			richTextBox1.Font = new Font("Courier New", 8.25F);
			richTextBox1.Location = new Point(0, 0);
			richTextBox1.Margin = new Padding(5, 4, 5, 4);
			richTextBox1.Name = "richTextBox1";
			richTextBox1.Size = new Size(1807, 701);
			richTextBox1.TabIndex = 0;
			richTextBox1.Text = "";
			// 
			// button2
			// 
			button2.Location = new Point(11, 4);
			button2.Margin = new Padding(5, 4, 5, 4);
			button2.Name = "button2";
			button2.Size = new Size(101, 36);
			button2.TabIndex = 0;
			button2.Text = "Assemble";
			button2.UseVisualStyleBackColor = true;
			// 
			// tabPage5
			// 
			tabPage5.Controls.Add(wavesControl1);
			tabPage5.Controls.Add(toolStrip2);
			tabPage5.Location = new Point(4, 29);
			tabPage5.Margin = new Padding(3, 4, 3, 4);
			tabPage5.Name = "tabPage5";
			tabPage5.Size = new Size(1817, 800);
			tabPage5.TabIndex = 3;
			tabPage5.Text = "Waves";
			tabPage5.UseVisualStyleBackColor = true;
			// 
			// wavesControl1
			// 
			wavesControl1.Dock = DockStyle.Fill;
			wavesControl1.Font = new Font("Segoe UI", 6F);
			wavesControl1.Location = new Point(0, 27);
			wavesControl1.Margin = new Padding(3, 4, 3, 4);
			wavesControl1.Name = "wavesControl1";
			wavesControl1.Size = new Size(1817, 773);
			wavesControl1.TabIndex = 1;
			wavesControl1.Text = "wavesControl1";
			// 
			// toolStrip2
			// 
			toolStrip2.ImageScalingSize = new Size(20, 20);
			toolStrip2.Items.AddRange(new ToolStripItem[] { toolStripButton7, toolStripButton8, toolStripSeparator6, toolStripButton9, toolStripButton10, toolStripButton11, toolStripButton12, toolStripButton13 });
			toolStrip2.Location = new Point(0, 0);
			toolStrip2.Name = "toolStrip2";
			toolStrip2.Size = new Size(1817, 27);
			toolStrip2.TabIndex = 0;
			toolStrip2.Text = "toolStrip2";
			// 
			// toolStripButton7
			// 
			toolStripButton7.Alignment = ToolStripItemAlignment.Right;
			toolStripButton7.DisplayStyle = ToolStripItemDisplayStyle.Text;
			toolStripButton7.Image = (Image)resources.GetObject("toolStripButton7.Image");
			toolStripButton7.ImageTransparentColor = Color.Magenta;
			toolStripButton7.Name = "toolStripButton7";
			toolStripButton7.Size = new Size(57, 24);
			toolStripButton7.Text = "Snatch";
			toolStripButton7.Click += toolStripButton7_Click;
			// 
			// toolStripButton8
			// 
			toolStripButton8.DisplayStyle = ToolStripItemDisplayStyle.Text;
			toolStripButton8.Image = (Image)resources.GetObject("toolStripButton8.Image");
			toolStripButton8.ImageTransparentColor = Color.Magenta;
			toolStripButton8.Name = "toolStripButton8";
			toolStripButton8.Size = new Size(47, 24);
			toolStripButton8.Text = "Clear";
			toolStripButton8.Click += toolStripButton8_Click;
			// 
			// toolStripSeparator6
			// 
			toolStripSeparator6.Name = "toolStripSeparator6";
			toolStripSeparator6.Size = new Size(6, 27);
			// 
			// toolStripButton9
			// 
			toolStripButton9.DisplayStyle = ToolStripItemDisplayStyle.Text;
			toolStripButton9.Image = (Image)resources.GetObject("toolStripButton9.Image");
			toolStripButton9.ImageTransparentColor = Color.Magenta;
			toolStripButton9.Name = "toolStripButton9";
			toolStripButton9.Size = new Size(40, 24);
			toolStripButton9.Text = "BRK";
			toolStripButton9.Click += toolStripButton9_Click;
			// 
			// toolStripButton10
			// 
			toolStripButton10.DisplayStyle = ToolStripItemDisplayStyle.Text;
			toolStripButton10.Image = (Image)resources.GetObject("toolStripButton10.Image");
			toolStripButton10.ImageTransparentColor = Color.Magenta;
			toolStripButton10.Name = "toolStripButton10";
			toolStripButton10.Size = new Size(43, 24);
			toolStripButton10.Text = "Disp";
			toolStripButton10.Click += toolStripButton10_Click;
			// 
			// toolStripButton11
			// 
			toolStripButton11.DisplayStyle = ToolStripItemDisplayStyle.Text;
			toolStripButton11.Image = (Image)resources.GetObject("toolStripButton11.Image");
			toolStripButton11.ImageTransparentColor = Color.Magenta;
			toolStripButton11.Name = "toolStripButton11";
			toolStripButton11.Size = new Size(40, 24);
			toolStripButton11.Text = "ALU";
			toolStripButton11.Click += toolStripButton11_Click;
			// 
			// toolStripButton12
			// 
			toolStripButton12.DisplayStyle = ToolStripItemDisplayStyle.Text;
			toolStripButton12.Image = (Image)resources.GetObject("toolStripButton12.Image");
			toolStripButton12.ImageTransparentColor = Color.Magenta;
			toolStripButton12.Name = "toolStripButton12";
			toolStripButton12.Size = new Size(46, 24);
			toolStripButton12.Text = "Bops";
			toolStripButton12.Click += toolStripButton12_Click;
			// 
			// toolStripButton13
			// 
			toolStripButton13.DisplayStyle = ToolStripItemDisplayStyle.Text;
			toolStripButton13.Image = (Image)resources.GetObject("toolStripButton13.Image");
			toolStripButton13.ImageTransparentColor = Color.Magenta;
			toolStripButton13.Name = "toolStripButton13";
			toolStripButton13.Size = new Size(44, 24);
			toolStripButton13.Text = "Fops";
			toolStripButton13.Click += toolStripButton13_Click;
			// 
			// tabPage2
			// 
			tabPage2.Controls.Add(dataPathView21);
			tabPage2.Location = new Point(4, 29);
			tabPage2.Margin = new Padding(3, 4, 3, 4);
			tabPage2.Name = "tabPage2";
			tabPage2.Size = new Size(1817, 800);
			tabPage2.TabIndex = 4;
			tabPage2.Text = "DataPath";
			tabPage2.UseVisualStyleBackColor = true;
			// 
			// dataPathView21
			// 
			dataPathView21.BackgroundImage = Properties.Resources.datapath;
			dataPathView21.Dock = DockStyle.Fill;
			dataPathView21.Location = new Point(0, 0);
			dataPathView21.Margin = new Padding(3, 4, 3, 4);
			dataPathView21.Name = "dataPathView21";
			dataPathView21.Size = new Size(1817, 800);
			dataPathView21.TabIndex = 1;
			dataPathView21.Text = "dataPathView21";
			// 
			// tabPage6
			// 
			tabPage6.Controls.Add(hexBox1);
			tabPage6.Location = new Point(4, 29);
			tabPage6.Margin = new Padding(3, 4, 3, 4);
			tabPage6.Name = "tabPage6";
			tabPage6.Size = new Size(1817, 800);
			tabPage6.TabIndex = 5;
			tabPage6.Text = "CPU Memory";
			tabPage6.UseVisualStyleBackColor = true;
			// 
			// hexBox1
			// 
			hexBox1.ColumnInfoVisible = true;
			hexBox1.Dock = DockStyle.Fill;
			hexBox1.Font = new Font("Segoe UI", 9F);
			hexBox1.LineInfoVisible = true;
			hexBox1.Location = new Point(0, 0);
			hexBox1.Margin = new Padding(5, 4, 5, 4);
			hexBox1.Name = "hexBox1";
			hexBox1.ReadOnly = true;
			hexBox1.ShadowSelectionColor = Color.FromArgb(100, 60, 188, 255);
			hexBox1.Size = new Size(1817, 800);
			hexBox1.StringViewVisible = true;
			hexBox1.TabIndex = 3;
			hexBox1.UseFixedBytesPerLine = true;
			hexBox1.VScrollBarVisible = true;
			// 
			// FormMain
			// 
			AutoScaleDimensions = new SizeF(8F, 20F);
			AutoScaleMode = AutoScaleMode.Font;
			ClientSize = new Size(1825, 928);
			Controls.Add(tabControl2);
			Controls.Add(statusStrip1);
			Controls.Add(toolStrip1);
			Controls.Add(menuStrip1);
			KeyPreview = true;
			MainMenuStrip = menuStrip1;
			Margin = new Padding(5, 4, 5, 4);
			MinimumSize = new Size(1274, 854);
			Name = "FormMain";
			Text = "Breaks Debugger";
			Load += Form1_Load;
			KeyDown += Form1_KeyDown;
			menuStrip1.ResumeLayout(false);
			menuStrip1.PerformLayout();
			toolStrip1.ResumeLayout(false);
			toolStrip1.PerformLayout();
			statusStrip1.ResumeLayout(false);
			statusStrip1.PerformLayout();
			tabControl2.ResumeLayout(false);
			tabPage3.ResumeLayout(false);
			tableLayoutPanel1.ResumeLayout(false);
			tableLayoutPanel1.PerformLayout();
			splitContainer4.Panel1.ResumeLayout(false);
			splitContainer4.Panel1.PerformLayout();
			splitContainer4.Panel2.ResumeLayout(false);
			splitContainer4.Panel2.PerformLayout();
			((System.ComponentModel.ISupportInitialize)splitContainer4).EndInit();
			splitContainer4.ResumeLayout(false);
			splitContainer2.Panel1.ResumeLayout(false);
			splitContainer2.Panel1.PerformLayout();
			splitContainer2.Panel2.ResumeLayout(false);
			splitContainer2.Panel2.PerformLayout();
			((System.ComponentModel.ISupportInitialize)splitContainer2).EndInit();
			splitContainer2.ResumeLayout(false);
			tabPage4.ResumeLayout(false);
			splitContainer3.Panel1.ResumeLayout(false);
			splitContainer3.Panel2.ResumeLayout(false);
			((System.ComponentModel.ISupportInitialize)splitContainer3).EndInit();
			splitContainer3.ResumeLayout(false);
			tabPage5.ResumeLayout(false);
			tabPage5.PerformLayout();
			toolStrip2.ResumeLayout(false);
			toolStrip2.PerformLayout();
			tabPage2.ResumeLayout(false);
			tabPage6.ResumeLayout(false);
			ResumeLayout(false);
			PerformLayout();
		}

		#endregion

		private System.Windows.Forms.MenuStrip menuStrip1;
		private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem helpToolStripMenuItem;
		private System.Windows.Forms.ToolStrip toolStrip1;
		private System.Windows.Forms.ToolStripButton toolStripButton1;
		private System.Windows.Forms.StatusStrip statusStrip1;
		private System.Windows.Forms.ToolStripStatusLabel toolStripStatusLabel1;
		private System.Windows.Forms.ToolStripMenuItem loadMemoryDumpToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem saveTheMemoryDumpToolStripMenuItem;
		private System.Windows.Forms.ToolStripSeparator toolStripSeparator1;
		private System.Windows.Forms.ToolStripMenuItem exitToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem aboutToolStripMenuItem;
		private System.Windows.Forms.ToolStripSeparator toolStripSeparator2;
		private System.Windows.Forms.ToolStripButton toolStripButton2;
		private System.Windows.Forms.ToolStripButton toolStripButton3;
		private System.Windows.Forms.ToolStripSeparator toolStripSeparator3;
		private System.Windows.Forms.ToolStripButton toolStripButton4;
		private System.Windows.Forms.ToolStripButton toolStripButton5;
		private System.Windows.Forms.ToolStripButton toolStripButton6;
		private System.Windows.Forms.ToolStripSeparator toolStripSeparator4;
		private System.Windows.Forms.ToolStripMenuItem loadAssemblySourceToolStripMenuItem;
		private System.Windows.Forms.OpenFileDialog openFileDialog1;
		private System.Windows.Forms.OpenFileDialog openFileDialog2;
		private System.Windows.Forms.SaveFileDialog saveFileDialog1;
		private ToolStripMenuItem sendFeedbackToolStripMenuItem;
		private ToolStripMenuItem checkForUpdatesToolStripMenuItem;
		private ToolStripSeparator toolStripSeparator5;
		private TabControl tabControl2;
		private TabPage tabPage3;
		private TableLayoutPanel tableLayoutPanel1;
		private SplitContainer splitContainer4;
		private Label label1;
		private Label label9;
		private PropertyGrid propertyGrid1;
		private PropertyGrid propertyGrid2;
		private PropertyGrid propertyGrid3;
		private PropertyGrid propertyGrid4;
		private PropertyGrid propertyGrid5;
		private Label label5;
		private Label label6;
		private Label label7;
		private Label label8;
		private SplitContainer splitContainer2;
		private Label label2;
		private Label label3;
		private Label label4;
		private TabPage tabPage4;
		private SplitContainer splitContainer3;
		private RichTextBox richTextBox1;
		private Button button2;
		private TabPage tabPage5;
		private WavesControl wavesControl1;
		private ToolStrip toolStrip2;
		private ToolStripButton toolStripButton7;
		private ToolStripButton toolStripButton8;
		private ToolStripSeparator toolStripSeparator6;
		private ToolStripButton toolStripButton9;
		private ToolStripButton toolStripButton10;
		private ToolStripButton toolStripButton11;
		private ToolStripButton toolStripButton12;
		private ToolStripButton toolStripButton13;
		private TabPage tabPage2;
		private DataPathView dataPathView21;
		private TabPage tabPage6;
		private Be.Windows.Forms.HexBox hexBox1;
	}
}

