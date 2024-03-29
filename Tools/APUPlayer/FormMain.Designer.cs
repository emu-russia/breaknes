﻿namespace APUPlayer
{
	partial class FormMain
	{
		/// <summary>
		///  Required designer variable.
		/// </summary>
		private System.ComponentModel.IContainer components = null;

		/// <summary>
		///  Clean up any resources being used.
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
		///  Required method for Designer support - do not modify
		///  the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(FormMain));
			menuStrip1 = new MenuStrip();
			fileToolStripMenuItem = new ToolStripMenuItem();
			loadAPURegisterDumpToolStripMenuItem = new ToolStripMenuItem();
			loadAUXDumpToolStripMenuItem = new ToolStripMenuItem();
			loadCPUMemoryDumpToolStripMenuItem = new ToolStripMenuItem();
			saveSampleBufferAsWAVToolStripMenuItem = new ToolStripMenuItem();
			toolStripSeparator1 = new ToolStripSeparator();
			settingsToolStripMenuItem = new ToolStripMenuItem();
			exitToolStripMenuItem = new ToolStripMenuItem();
			debugToolStripMenuItem = new ToolStripMenuItem();
			loadSaveLogisimAUXDumpForAudacityToolStripMenuItem = new ToolStripMenuItem();
			controlsToolStripMenuItem = new ToolStripMenuItem();
			playToolStripMenuItem = new ToolStripMenuItem();
			pauseToolStripMenuItem = new ToolStripMenuItem();
			stopToolStripMenuItem = new ToolStripMenuItem();
			helpToolStripMenuItem = new ToolStripMenuItem();
			sendFeedbackToolStripMenuItem = new ToolStripMenuItem();
			checkForUpdatesToolStripMenuItem = new ToolStripMenuItem();
			toolStripSeparator3 = new ToolStripSeparator();
			aboutToolStripMenuItem = new ToolStripMenuItem();
			statusStrip1 = new StatusStrip();
			toolStripStatusLabel1 = new ToolStripStatusLabel();
			toolStripStatusLabelSamples = new ToolStripStatusLabel();
			toolStripStatusLabel3 = new ToolStripStatusLabel();
			toolStripStatusLabelMsec = new ToolStripStatusLabel();
			toolStripStatusLabel5 = new ToolStripStatusLabel();
			toolStripStatusLabelState = new ToolStripStatusLabel();
			toolStripStatusLabel6 = new ToolStripStatusLabel();
			toolStripStatusLabelACLKCount = new ToolStripStatusLabel();
			toolStripStatusLabel7 = new ToolStripStatusLabel();
			toolStripStatusLabelACLK = new ToolStripStatusLabel();
			openFileDialogNSF = new OpenFileDialog();
			openFileDialogHEX = new OpenFileDialog();
			saveFileDialogBin = new SaveFileDialog();
			toolStrip1 = new ToolStrip();
			toolStripButtonPlay = new ToolStripButton();
			toolStripButtonDiscard = new ToolStripButton();
			toolStripButtonStop = new ToolStripButton();
			toolStripSeparator4 = new ToolStripSeparator();
			toolStripButton1 = new ToolStripButton();
			toolStripButton2 = new ToolStripButton();
			tabControl1 = new TabControl();
			tabPage1 = new TabPage();
			splitContainer1 = new SplitContainer();
			signalPlot1 = new SignalPlot();
			furryPlot1 = new FurryPlot();
			tabPage2 = new TabPage();
			splitContainer2 = new SplitContainer();
			splitContainer3 = new SplitContainer();
			button2 = new Button();
			comboBox2 = new ComboBox();
			propertyGrid1 = new PropertyGrid();
			splitContainer4 = new SplitContainer();
			button3 = new Button();
			button1 = new Button();
			comboBox1 = new ComboBox();
			tabControl2 = new TabControl();
			tabPage3 = new TabPage();
			hexBox1 = new Be.Windows.Forms.HexBox();
			tabPage4 = new TabPage();
			wavesControl1 = new WavesControl();
			toolStrip2 = new ToolStrip();
			toolStripButton3 = new ToolStripButton();
			toolStripButton4 = new ToolStripButton();
			toolStripSeparator5 = new ToolStripSeparator();
			toolStripButton5 = new ToolStripButton();
			toolStripButton6 = new ToolStripButton();
			toolStripButton7 = new ToolStripButton();
			toolStripButton8 = new ToolStripButton();
			toolStripButton9 = new ToolStripButton();
			toolStripButton10 = new ToolStripButton();
			backgroundWorker1 = new System.ComponentModel.BackgroundWorker();
			openFileDialogRegDump = new OpenFileDialog();
			saveFileDialogWAV = new SaveFileDialog();
			openFileDialogCpumem = new OpenFileDialog();
			menuStrip1.SuspendLayout();
			statusStrip1.SuspendLayout();
			toolStrip1.SuspendLayout();
			tabControl1.SuspendLayout();
			tabPage1.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)splitContainer1).BeginInit();
			splitContainer1.Panel1.SuspendLayout();
			splitContainer1.Panel2.SuspendLayout();
			splitContainer1.SuspendLayout();
			tabPage2.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)splitContainer2).BeginInit();
			splitContainer2.Panel1.SuspendLayout();
			splitContainer2.Panel2.SuspendLayout();
			splitContainer2.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)splitContainer3).BeginInit();
			splitContainer3.Panel1.SuspendLayout();
			splitContainer3.Panel2.SuspendLayout();
			splitContainer3.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)splitContainer4).BeginInit();
			splitContainer4.Panel1.SuspendLayout();
			splitContainer4.Panel2.SuspendLayout();
			splitContainer4.SuspendLayout();
			tabControl2.SuspendLayout();
			tabPage3.SuspendLayout();
			tabPage4.SuspendLayout();
			toolStrip2.SuspendLayout();
			SuspendLayout();
			// 
			// menuStrip1
			// 
			menuStrip1.Items.AddRange(new ToolStripItem[] { fileToolStripMenuItem, debugToolStripMenuItem, controlsToolStripMenuItem, helpToolStripMenuItem });
			menuStrip1.Location = new Point(0, 0);
			menuStrip1.Name = "menuStrip1";
			menuStrip1.Size = new Size(1018, 24);
			menuStrip1.TabIndex = 0;
			menuStrip1.Text = "menuStrip1";
			// 
			// fileToolStripMenuItem
			// 
			fileToolStripMenuItem.DropDownItems.AddRange(new ToolStripItem[] { loadAPURegisterDumpToolStripMenuItem, loadAUXDumpToolStripMenuItem, loadCPUMemoryDumpToolStripMenuItem, saveSampleBufferAsWAVToolStripMenuItem, toolStripSeparator1, settingsToolStripMenuItem, exitToolStripMenuItem });
			fileToolStripMenuItem.Name = "fileToolStripMenuItem";
			fileToolStripMenuItem.Size = new Size(37, 20);
			fileToolStripMenuItem.Text = "File";
			// 
			// loadAPURegisterDumpToolStripMenuItem
			// 
			loadAPURegisterDumpToolStripMenuItem.Name = "loadAPURegisterDumpToolStripMenuItem";
			loadAPURegisterDumpToolStripMenuItem.Size = new Size(225, 22);
			loadAPURegisterDumpToolStripMenuItem.Text = "Load APU registers dump...";
			loadAPURegisterDumpToolStripMenuItem.Click += loadAPURegisterDumpToolStripMenuItem_Click;
			// 
			// loadAUXDumpToolStripMenuItem
			// 
			loadAUXDumpToolStripMenuItem.Name = "loadAUXDumpToolStripMenuItem";
			loadAUXDumpToolStripMenuItem.Size = new Size(225, 22);
			loadAUXDumpToolStripMenuItem.Text = "Load AUX dump...";
			loadAUXDumpToolStripMenuItem.Click += loadAUXDumpToolStripMenuItem_Click;
			// 
			// loadCPUMemoryDumpToolStripMenuItem
			// 
			loadCPUMemoryDumpToolStripMenuItem.Name = "loadCPUMemoryDumpToolStripMenuItem";
			loadCPUMemoryDumpToolStripMenuItem.Size = new Size(225, 22);
			loadCPUMemoryDumpToolStripMenuItem.Text = "Load CPU Memory dump...";
			loadCPUMemoryDumpToolStripMenuItem.Click += loadCPUMemoryDumpToolStripMenuItem_Click;
			// 
			// saveSampleBufferAsWAVToolStripMenuItem
			// 
			saveSampleBufferAsWAVToolStripMenuItem.Name = "saveSampleBufferAsWAVToolStripMenuItem";
			saveSampleBufferAsWAVToolStripMenuItem.Size = new Size(225, 22);
			saveSampleBufferAsWAVToolStripMenuItem.Text = "Save sample buffer as WAV...";
			saveSampleBufferAsWAVToolStripMenuItem.Click += saveSampleBufferAsWAVToolStripMenuItem_Click;
			// 
			// toolStripSeparator1
			// 
			toolStripSeparator1.Name = "toolStripSeparator1";
			toolStripSeparator1.Size = new Size(222, 6);
			// 
			// settingsToolStripMenuItem
			// 
			settingsToolStripMenuItem.Name = "settingsToolStripMenuItem";
			settingsToolStripMenuItem.Size = new Size(225, 22);
			settingsToolStripMenuItem.Text = "Settings...";
			settingsToolStripMenuItem.Click += settingsToolStripMenuItem_Click;
			// 
			// exitToolStripMenuItem
			// 
			exitToolStripMenuItem.Name = "exitToolStripMenuItem";
			exitToolStripMenuItem.Size = new Size(225, 22);
			exitToolStripMenuItem.Text = "Exit";
			exitToolStripMenuItem.Click += exitToolStripMenuItem_Click;
			// 
			// debugToolStripMenuItem
			// 
			debugToolStripMenuItem.DropDownItems.AddRange(new ToolStripItem[] { loadSaveLogisimAUXDumpForAudacityToolStripMenuItem });
			debugToolStripMenuItem.Name = "debugToolStripMenuItem";
			debugToolStripMenuItem.Size = new Size(54, 20);
			debugToolStripMenuItem.Text = "Debug";
			// 
			// loadSaveLogisimAUXDumpForAudacityToolStripMenuItem
			// 
			loadSaveLogisimAUXDumpForAudacityToolStripMenuItem.Name = "loadSaveLogisimAUXDumpForAudacityToolStripMenuItem";
			loadSaveLogisimAUXDumpForAudacityToolStripMenuItem.Size = new Size(313, 22);
			loadSaveLogisimAUXDumpForAudacityToolStripMenuItem.Text = "Load/Save Logisim AUX Dump for Audacity...";
			loadSaveLogisimAUXDumpForAudacityToolStripMenuItem.Click += loadSaveLogisimAUXDumpForAudacityToolStripMenuItem_Click;
			// 
			// controlsToolStripMenuItem
			// 
			controlsToolStripMenuItem.DropDownItems.AddRange(new ToolStripItem[] { playToolStripMenuItem, pauseToolStripMenuItem, stopToolStripMenuItem });
			controlsToolStripMenuItem.Name = "controlsToolStripMenuItem";
			controlsToolStripMenuItem.Size = new Size(105, 20);
			controlsToolStripMenuItem.Text = "Worker Controls";
			// 
			// playToolStripMenuItem
			// 
			playToolStripMenuItem.Name = "playToolStripMenuItem";
			playToolStripMenuItem.Size = new Size(105, 22);
			playToolStripMenuItem.Text = "Run";
			playToolStripMenuItem.Click += playToolStripMenuItem_Click;
			// 
			// pauseToolStripMenuItem
			// 
			pauseToolStripMenuItem.Name = "pauseToolStripMenuItem";
			pauseToolStripMenuItem.Size = new Size(105, 22);
			pauseToolStripMenuItem.Text = "Pause";
			pauseToolStripMenuItem.Click += pauseToolStripMenuItem_Click;
			// 
			// stopToolStripMenuItem
			// 
			stopToolStripMenuItem.Name = "stopToolStripMenuItem";
			stopToolStripMenuItem.Size = new Size(105, 22);
			stopToolStripMenuItem.Text = "Stop";
			stopToolStripMenuItem.Click += stopToolStripMenuItem_Click;
			// 
			// helpToolStripMenuItem
			// 
			helpToolStripMenuItem.DropDownItems.AddRange(new ToolStripItem[] { sendFeedbackToolStripMenuItem, checkForUpdatesToolStripMenuItem, toolStripSeparator3, aboutToolStripMenuItem });
			helpToolStripMenuItem.Name = "helpToolStripMenuItem";
			helpToolStripMenuItem.Size = new Size(44, 20);
			helpToolStripMenuItem.Text = "Help";
			// 
			// sendFeedbackToolStripMenuItem
			// 
			sendFeedbackToolStripMenuItem.Name = "sendFeedbackToolStripMenuItem";
			sendFeedbackToolStripMenuItem.Size = new Size(170, 22);
			sendFeedbackToolStripMenuItem.Text = "Send feedback";
			sendFeedbackToolStripMenuItem.Click += sendFeedbackToolStripMenuItem_Click;
			// 
			// checkForUpdatesToolStripMenuItem
			// 
			checkForUpdatesToolStripMenuItem.Name = "checkForUpdatesToolStripMenuItem";
			checkForUpdatesToolStripMenuItem.Size = new Size(170, 22);
			checkForUpdatesToolStripMenuItem.Text = "Check for updates";
			checkForUpdatesToolStripMenuItem.Click += checkForUpdatesToolStripMenuItem_Click;
			// 
			// toolStripSeparator3
			// 
			toolStripSeparator3.Name = "toolStripSeparator3";
			toolStripSeparator3.Size = new Size(167, 6);
			// 
			// aboutToolStripMenuItem
			// 
			aboutToolStripMenuItem.Name = "aboutToolStripMenuItem";
			aboutToolStripMenuItem.Size = new Size(170, 22);
			aboutToolStripMenuItem.Text = "About";
			aboutToolStripMenuItem.Click += aboutToolStripMenuItem_Click;
			// 
			// statusStrip1
			// 
			statusStrip1.Items.AddRange(new ToolStripItem[] { toolStripStatusLabel1, toolStripStatusLabelSamples, toolStripStatusLabel3, toolStripStatusLabelMsec, toolStripStatusLabel5, toolStripStatusLabelState, toolStripStatusLabel6, toolStripStatusLabelACLKCount, toolStripStatusLabel7, toolStripStatusLabelACLK });
			statusStrip1.Location = new Point(0, 533);
			statusStrip1.Name = "statusStrip1";
			statusStrip1.Size = new Size(1018, 24);
			statusStrip1.TabIndex = 1;
			statusStrip1.Text = "statusStrip1";
			// 
			// toolStripStatusLabel1
			// 
			toolStripStatusLabel1.Name = "toolStripStatusLabel1";
			toolStripStatusLabel1.Size = new Size(105, 19);
			toolStripStatusLabel1.Text = "Samples collected:";
			// 
			// toolStripStatusLabelSamples
			// 
			toolStripStatusLabelSamples.BorderSides = ToolStripStatusLabelBorderSides.Right;
			toolStripStatusLabelSamples.Name = "toolStripStatusLabelSamples";
			toolStripStatusLabelSamples.Size = new Size(17, 19);
			toolStripStatusLabelSamples.Text = "0";
			// 
			// toolStripStatusLabel3
			// 
			toolStripStatusLabel3.Name = "toolStripStatusLabel3";
			toolStripStatusLabel3.Size = new Size(91, 19);
			toolStripStatusLabel3.Text = "Samples length:";
			// 
			// toolStripStatusLabelMsec
			// 
			toolStripStatusLabelMsec.BorderSides = ToolStripStatusLabelBorderSides.Right;
			toolStripStatusLabelMsec.Name = "toolStripStatusLabelMsec";
			toolStripStatusLabelMsec.Size = new Size(36, 19);
			toolStripStatusLabelMsec.Text = "0 ms";
			// 
			// toolStripStatusLabel5
			// 
			toolStripStatusLabel5.Name = "toolStripStatusLabel5";
			toolStripStatusLabel5.Size = new Size(48, 19);
			toolStripStatusLabel5.Text = "Worker:";
			// 
			// toolStripStatusLabelState
			// 
			toolStripStatusLabelState.BorderSides = ToolStripStatusLabelBorderSides.Right;
			toolStripStatusLabelState.Name = "toolStripStatusLabelState";
			toolStripStatusLabelState.Size = new Size(49, 19);
			toolStripStatusLabelState.Text = "Paused";
			// 
			// toolStripStatusLabel6
			// 
			toolStripStatusLabel6.Name = "toolStripStatusLabel6";
			toolStripStatusLabel6.Size = new Size(75, 19);
			toolStripStatusLabel6.Text = "ACLK Count:";
			// 
			// toolStripStatusLabelACLKCount
			// 
			toolStripStatusLabelACLKCount.BorderSides = ToolStripStatusLabelBorderSides.Right;
			toolStripStatusLabelACLKCount.Name = "toolStripStatusLabelACLKCount";
			toolStripStatusLabelACLKCount.Size = new Size(17, 19);
			toolStripStatusLabelACLKCount.Text = "0";
			// 
			// toolStripStatusLabel7
			// 
			toolStripStatusLabel7.Name = "toolStripStatusLabel7";
			toolStripStatusLabel7.Size = new Size(62, 19);
			toolStripStatusLabel7.Text = "ACLK/Sec:";
			// 
			// toolStripStatusLabelACLK
			// 
			toolStripStatusLabelACLK.Name = "toolStripStatusLabelACLK";
			toolStripStatusLabelACLK.Size = new Size(13, 19);
			toolStripStatusLabelACLK.Text = "0";
			// 
			// openFileDialogNSF
			// 
			openFileDialogNSF.DefaultExt = "nsf";
			openFileDialogNSF.Filter = "NSF files|*.nsf|All files|*.*";
			// 
			// openFileDialogHEX
			// 
			openFileDialogHEX.DefaultExt = "hex";
			openFileDialogHEX.Filter = "Logisim HEX files|*.hex|All files|*.*";
			// 
			// saveFileDialogBin
			// 
			saveFileDialogBin.DefaultExt = "bin";
			saveFileDialogBin.Filter = "Binary files|*.bin|All files|*.*";
			// 
			// toolStrip1
			// 
			toolStrip1.Items.AddRange(new ToolStripItem[] { toolStripButtonPlay, toolStripButtonDiscard, toolStripButtonStop, toolStripSeparator4, toolStripButton1, toolStripButton2 });
			toolStrip1.Location = new Point(0, 24);
			toolStrip1.Name = "toolStrip1";
			toolStrip1.Size = new Size(1018, 25);
			toolStrip1.TabIndex = 2;
			toolStrip1.Text = "toolStrip1";
			// 
			// toolStripButtonPlay
			// 
			toolStripButtonPlay.DisplayStyle = ToolStripItemDisplayStyle.Text;
			toolStripButtonPlay.Image = (Image)resources.GetObject("toolStripButtonPlay.Image");
			toolStripButtonPlay.ImageTransparentColor = Color.Magenta;
			toolStripButtonPlay.Name = "toolStripButtonPlay";
			toolStripButtonPlay.Size = new Size(33, 22);
			toolStripButtonPlay.Text = "Play";
			toolStripButtonPlay.Click += toolStripButtonPlay_Click;
			// 
			// toolStripButtonDiscard
			// 
			toolStripButtonDiscard.DisplayStyle = ToolStripItemDisplayStyle.Text;
			toolStripButtonDiscard.Image = (Image)resources.GetObject("toolStripButtonDiscard.Image");
			toolStripButtonDiscard.ImageTransparentColor = Color.Magenta;
			toolStripButtonDiscard.Name = "toolStripButtonDiscard";
			toolStripButtonDiscard.Size = new Size(50, 22);
			toolStripButtonDiscard.Text = "Discard";
			toolStripButtonDiscard.Click += toolStripButtonDiscard_Click;
			// 
			// toolStripButtonStop
			// 
			toolStripButtonStop.DisplayStyle = ToolStripItemDisplayStyle.Text;
			toolStripButtonStop.Image = (Image)resources.GetObject("toolStripButtonStop.Image");
			toolStripButtonStop.ImageTransparentColor = Color.Magenta;
			toolStripButtonStop.Name = "toolStripButtonStop";
			toolStripButtonStop.Size = new Size(35, 22);
			toolStripButtonStop.Text = "Stop";
			toolStripButtonStop.Click += toolStripButtonStop_Click;
			// 
			// toolStripSeparator4
			// 
			toolStripSeparator4.Name = "toolStripSeparator4";
			toolStripSeparator4.Size = new Size(6, 25);
			// 
			// toolStripButton1
			// 
			toolStripButton1.DisplayStyle = ToolStripItemDisplayStyle.Text;
			toolStripButton1.Image = (Image)resources.GetObject("toolStripButton1.Image");
			toolStripButton1.ImageTransparentColor = Color.Magenta;
			toolStripButton1.Name = "toolStripButton1";
			toolStripButton1.Size = new Size(38, 22);
			toolStripButton1.Text = "Furry";
			toolStripButton1.ToolTipText = "Enable FFT";
			toolStripButton1.Click += toolStripButton1_Click;
			// 
			// toolStripButton2
			// 
			toolStripButton2.Alignment = ToolStripItemAlignment.Right;
			toolStripButton2.DisplayStyle = ToolStripItemDisplayStyle.Text;
			toolStripButton2.Image = (Image)resources.GetObject("toolStripButton2.Image");
			toolStripButton2.ImageTransparentColor = Color.Magenta;
			toolStripButton2.Name = "toolStripButton2";
			toolStripButton2.Size = new Size(47, 22);
			toolStripButton2.Text = "Snatch";
			toolStripButton2.Click += toolStripButton2_Click;
			// 
			// tabControl1
			// 
			tabControl1.Controls.Add(tabPage1);
			tabControl1.Controls.Add(tabPage2);
			tabControl1.Dock = DockStyle.Fill;
			tabControl1.Location = new Point(0, 49);
			tabControl1.Name = "tabControl1";
			tabControl1.SelectedIndex = 0;
			tabControl1.Size = new Size(1018, 484);
			tabControl1.TabIndex = 3;
			// 
			// tabPage1
			// 
			tabPage1.Controls.Add(splitContainer1);
			tabPage1.Location = new Point(4, 24);
			tabPage1.Name = "tabPage1";
			tabPage1.Padding = new Padding(3);
			tabPage1.Size = new Size(1010, 456);
			tabPage1.TabIndex = 0;
			tabPage1.Text = "Wave Dump";
			tabPage1.UseVisualStyleBackColor = true;
			// 
			// splitContainer1
			// 
			splitContainer1.Dock = DockStyle.Fill;
			splitContainer1.Location = new Point(3, 3);
			splitContainer1.Name = "splitContainer1";
			splitContainer1.Orientation = Orientation.Horizontal;
			// 
			// splitContainer1.Panel1
			// 
			splitContainer1.Panel1.Controls.Add(signalPlot1);
			// 
			// splitContainer1.Panel2
			// 
			splitContainer1.Panel2.Controls.Add(furryPlot1);
			splitContainer1.Size = new Size(1004, 450);
			splitContainer1.SplitterDistance = 261;
			splitContainer1.TabIndex = 0;
			// 
			// signalPlot1
			// 
			signalPlot1.Dock = DockStyle.Fill;
			signalPlot1.DottedColor = Color.Silver;
			signalPlot1.DottedOpacity = 0;
			signalPlot1.FillColor = Color.Black;
			signalPlot1.Font = new Font("Segoe UI", 8.25F, FontStyle.Regular, GraphicsUnit.Point);
			signalPlot1.GridColor = Color.Green;
			signalPlot1.GridOpacity = 100;
			signalPlot1.LabelsColor = Color.White;
			signalPlot1.Location = new Point(0, 0);
			signalPlot1.Name = "signalPlot1";
			signalPlot1.SelectionColor = Color.GhostWhite;
			signalPlot1.SignalColor = Color.LightGreen;
			signalPlot1.Size = new Size(1004, 261);
			signalPlot1.TabIndex = 0;
			signalPlot1.Text = "signalPlot1";
			signalPlot1.ZeroColor = Color.Red;
			// 
			// furryPlot1
			// 
			furryPlot1.Dock = DockStyle.Fill;
			furryPlot1.Location = new Point(0, 0);
			furryPlot1.Name = "furryPlot1";
			furryPlot1.Size = new Size(1004, 185);
			furryPlot1.TabIndex = 0;
			furryPlot1.Text = "furryPlot1";
			// 
			// tabPage2
			// 
			tabPage2.Controls.Add(splitContainer2);
			tabPage2.Location = new Point(4, 24);
			tabPage2.Name = "tabPage2";
			tabPage2.Padding = new Padding(3);
			tabPage2.Size = new Size(1010, 456);
			tabPage2.TabIndex = 1;
			tabPage2.Text = "APU Debug";
			tabPage2.UseVisualStyleBackColor = true;
			// 
			// splitContainer2
			// 
			splitContainer2.Dock = DockStyle.Fill;
			splitContainer2.Location = new Point(3, 3);
			splitContainer2.Name = "splitContainer2";
			// 
			// splitContainer2.Panel1
			// 
			splitContainer2.Panel1.Controls.Add(splitContainer3);
			// 
			// splitContainer2.Panel2
			// 
			splitContainer2.Panel2.Controls.Add(splitContainer4);
			splitContainer2.Size = new Size(1004, 450);
			splitContainer2.SplitterDistance = 334;
			splitContainer2.TabIndex = 1;
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
			splitContainer3.Size = new Size(334, 450);
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
			comboBox2.Items.AddRange(new object[] { "Core", "CoreRegs", "APU", "APURegs", "Board" });
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
			propertyGrid1.Size = new Size(334, 404);
			propertyGrid1.TabIndex = 1;
			// 
			// splitContainer4
			// 
			splitContainer4.Dock = DockStyle.Fill;
			splitContainer4.FixedPanel = FixedPanel.Panel1;
			splitContainer4.Location = new Point(0, 0);
			splitContainer4.Name = "splitContainer4";
			splitContainer4.Orientation = Orientation.Horizontal;
			// 
			// splitContainer4.Panel1
			// 
			splitContainer4.Panel1.Controls.Add(button3);
			splitContainer4.Panel1.Controls.Add(button1);
			splitContainer4.Panel1.Controls.Add(comboBox1);
			// 
			// splitContainer4.Panel2
			// 
			splitContainer4.Panel2.Controls.Add(tabControl2);
			splitContainer4.Size = new Size(666, 450);
			splitContainer4.SplitterDistance = 42;
			splitContainer4.TabIndex = 0;
			// 
			// button3
			// 
			button3.Location = new Point(9, 5);
			button3.Name = "button3";
			button3.Size = new Size(101, 32);
			button3.TabIndex = 2;
			button3.Text = "Step (F11)";
			button3.UseVisualStyleBackColor = true;
			button3.Click += button3_Click;
			// 
			// button1
			// 
			button1.Anchor = AnchorStyles.Right;
			button1.BackgroundImageLayout = ImageLayout.Zoom;
			button1.Image = Properties.Resources.button_get_debug;
			button1.Location = new Point(627, 5);
			button1.Name = "button1";
			button1.Size = new Size(32, 32);
			button1.TabIndex = 1;
			button1.UseVisualStyleBackColor = true;
			button1.Click += button1_Click;
			// 
			// comboBox1
			// 
			comboBox1.Anchor = AnchorStyles.Right;
			comboBox1.DropDownStyle = ComboBoxStyle.DropDownList;
			comboBox1.FormattingEnabled = true;
			comboBox1.Location = new Point(408, 9);
			comboBox1.Name = "comboBox1";
			comboBox1.Size = new Size(213, 23);
			comboBox1.TabIndex = 0;
			comboBox1.SelectedIndexChanged += comboBox1_SelectedIndexChanged;
			// 
			// tabControl2
			// 
			tabControl2.Controls.Add(tabPage3);
			tabControl2.Controls.Add(tabPage4);
			tabControl2.Dock = DockStyle.Fill;
			tabControl2.Location = new Point(0, 0);
			tabControl2.Name = "tabControl2";
			tabControl2.SelectedIndex = 0;
			tabControl2.Size = new Size(666, 404);
			tabControl2.TabIndex = 0;
			// 
			// tabPage3
			// 
			tabPage3.Controls.Add(hexBox1);
			tabPage3.Location = new Point(4, 24);
			tabPage3.Name = "tabPage3";
			tabPage3.Padding = new Padding(3);
			tabPage3.Size = new Size(658, 376);
			tabPage3.TabIndex = 0;
			tabPage3.Text = "Hex";
			tabPage3.UseVisualStyleBackColor = true;
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
			hexBox1.Size = new Size(652, 370);
			hexBox1.StringViewVisible = true;
			hexBox1.TabIndex = 2;
			hexBox1.UseFixedBytesPerLine = true;
			hexBox1.VScrollBarVisible = true;
			// 
			// tabPage4
			// 
			tabPage4.Controls.Add(wavesControl1);
			tabPage4.Controls.Add(toolStrip2);
			tabPage4.Location = new Point(4, 24);
			tabPage4.Name = "tabPage4";
			tabPage4.Padding = new Padding(3);
			tabPage4.Size = new Size(658, 376);
			tabPage4.TabIndex = 1;
			tabPage4.Text = "Waves";
			tabPage4.UseVisualStyleBackColor = true;
			// 
			// wavesControl1
			// 
			wavesControl1.Dock = DockStyle.Fill;
			wavesControl1.DottedColor = Color.Silver;
			wavesControl1.DottedOpacity = 95;
			wavesControl1.FillColor = Color.Black;
			wavesControl1.Font = new Font("Segoe UI", 6F, FontStyle.Regular, GraphicsUnit.Point);
			wavesControl1.GridColor = Color.Green;
			wavesControl1.GridOpacity = 95;
			wavesControl1.HighZColor = Color.Gold;
			wavesControl1.LabelsColor = Color.White;
			wavesControl1.Location = new Point(3, 28);
			wavesControl1.Name = "wavesControl1";
			wavesControl1.SelectionColor = Color.GhostWhite;
			wavesControl1.SignalColor = Color.SpringGreen;
			wavesControl1.Size = new Size(652, 345);
			wavesControl1.TabIndex = 1;
			wavesControl1.Text = "wavesControl1";
			wavesControl1.UndefinedColor = Color.Red;
			// 
			// toolStrip2
			// 
			toolStrip2.Items.AddRange(new ToolStripItem[] { toolStripButton3, toolStripButton4, toolStripSeparator5, toolStripButton5, toolStripButton6, toolStripButton7, toolStripButton8, toolStripButton9, toolStripButton10 });
			toolStrip2.Location = new Point(3, 3);
			toolStrip2.Name = "toolStrip2";
			toolStrip2.Size = new Size(652, 25);
			toolStrip2.TabIndex = 0;
			toolStrip2.Text = "toolStrip2";
			// 
			// toolStripButton3
			// 
			toolStripButton3.Alignment = ToolStripItemAlignment.Right;
			toolStripButton3.DisplayStyle = ToolStripItemDisplayStyle.Text;
			toolStripButton3.Image = (Image)resources.GetObject("toolStripButton3.Image");
			toolStripButton3.ImageTransparentColor = Color.Magenta;
			toolStripButton3.Name = "toolStripButton3";
			toolStripButton3.Size = new Size(47, 22);
			toolStripButton3.Text = "Snatch";
			toolStripButton3.Click += toolStripButton3_Click;
			// 
			// toolStripButton4
			// 
			toolStripButton4.DisplayStyle = ToolStripItemDisplayStyle.Text;
			toolStripButton4.Image = (Image)resources.GetObject("toolStripButton4.Image");
			toolStripButton4.ImageTransparentColor = Color.Magenta;
			toolStripButton4.Name = "toolStripButton4";
			toolStripButton4.Size = new Size(38, 22);
			toolStripButton4.Text = "Clear";
			toolStripButton4.Click += toolStripButton4_Click;
			// 
			// toolStripSeparator5
			// 
			toolStripSeparator5.Name = "toolStripSeparator5";
			toolStripSeparator5.Size = new Size(6, 25);
			// 
			// toolStripButton5
			// 
			toolStripButton5.Checked = true;
			toolStripButton5.CheckState = CheckState.Checked;
			toolStripButton5.DisplayStyle = ToolStripItemDisplayStyle.Text;
			toolStripButton5.Image = (Image)resources.GetObject("toolStripButton5.Image");
			toolStripButton5.ImageTransparentColor = Color.Magenta;
			toolStripButton5.Name = "toolStripButton5";
			toolStripButton5.Size = new Size(46, 22);
			toolStripButton5.Text = "Clocks";
			toolStripButton5.Click += toolStripButton5_Click;
			// 
			// toolStripButton6
			// 
			toolStripButton6.DisplayStyle = ToolStripItemDisplayStyle.Text;
			toolStripButton6.Image = (Image)resources.GetObject("toolStripButton6.Image");
			toolStripButton6.ImageTransparentColor = Color.Magenta;
			toolStripButton6.Name = "toolStripButton6";
			toolStripButton6.Size = new Size(36, 22);
			toolStripButton6.Text = "Core";
			toolStripButton6.Click += toolStripButton6_Click;
			// 
			// toolStripButton7
			// 
			toolStripButton7.DisplayStyle = ToolStripItemDisplayStyle.Text;
			toolStripButton7.Image = (Image)resources.GetObject("toolStripButton7.Image");
			toolStripButton7.ImageTransparentColor = Color.Magenta;
			toolStripButton7.Name = "toolStripButton7";
			toolStripButton7.Size = new Size(38, 22);
			toolStripButton7.Text = "DMA";
			toolStripButton7.Click += toolStripButton7_Click;
			// 
			// toolStripButton8
			// 
			toolStripButton8.DisplayStyle = ToolStripItemDisplayStyle.Text;
			toolStripButton8.Image = (Image)resources.GetObject("toolStripButton8.Image");
			toolStripButton8.ImageTransparentColor = Color.Magenta;
			toolStripButton8.Name = "toolStripButton8";
			toolStripButton8.Size = new Size(52, 22);
			toolStripButton8.Text = "RegOps";
			toolStripButton8.Click += toolStripButton8_Click;
			// 
			// toolStripButton9
			// 
			toolStripButton9.DisplayStyle = ToolStripItemDisplayStyle.Text;
			toolStripButton9.Image = (Image)resources.GetObject("toolStripButton9.Image");
			toolStripButton9.ImageTransparentColor = Color.Magenta;
			toolStripButton9.Name = "toolStripButton9";
			toolStripButton9.Size = new Size(25, 22);
			toolStripButton9.Text = "LC";
			toolStripButton9.Click += toolStripButton9_Click;
			// 
			// toolStripButton10
			// 
			toolStripButton10.DisplayStyle = ToolStripItemDisplayStyle.Text;
			toolStripButton10.Image = (Image)resources.GetObject("toolStripButton10.Image");
			toolStripButton10.ImageTransparentColor = Color.Magenta;
			toolStripButton10.Name = "toolStripButton10";
			toolStripButton10.Size = new Size(46, 22);
			toolStripButton10.Text = "Others";
			toolStripButton10.Click += toolStripButton10_Click;
			// 
			// backgroundWorker1
			// 
			backgroundWorker1.WorkerSupportsCancellation = true;
			backgroundWorker1.DoWork += backgroundWorker1_DoWork;
			// 
			// openFileDialogRegDump
			// 
			openFileDialogRegDump.DefaultExt = "bin";
			openFileDialogRegDump.Filter = "Binary files|*.bin;*.regdump|All files|*.*";
			// 
			// saveFileDialogWAV
			// 
			saveFileDialogWAV.DefaultExt = "wav";
			saveFileDialogWAV.Filter = "Wave files|*.wav|All files|*.*";
			// 
			// openFileDialogCpumem
			// 
			openFileDialogCpumem.DefaultExt = "cpumem";
			openFileDialogCpumem.Filter = "Nintendulator CPU Memory dumps|*.cpumem|All files|*.*";
			// 
			// FormMain
			// 
			AutoScaleDimensions = new SizeF(7F, 15F);
			AutoScaleMode = AutoScaleMode.Font;
			ClientSize = new Size(1018, 557);
			Controls.Add(tabControl1);
			Controls.Add(toolStrip1);
			Controls.Add(statusStrip1);
			Controls.Add(menuStrip1);
			KeyPreview = true;
			MainMenuStrip = menuStrip1;
			Name = "FormMain";
			Text = "APU Player";
			Load += FormMain_Load;
			KeyDown += FormMain_KeyDown;
			menuStrip1.ResumeLayout(false);
			menuStrip1.PerformLayout();
			statusStrip1.ResumeLayout(false);
			statusStrip1.PerformLayout();
			toolStrip1.ResumeLayout(false);
			toolStrip1.PerformLayout();
			tabControl1.ResumeLayout(false);
			tabPage1.ResumeLayout(false);
			splitContainer1.Panel1.ResumeLayout(false);
			splitContainer1.Panel2.ResumeLayout(false);
			((System.ComponentModel.ISupportInitialize)splitContainer1).EndInit();
			splitContainer1.ResumeLayout(false);
			tabPage2.ResumeLayout(false);
			splitContainer2.Panel1.ResumeLayout(false);
			splitContainer2.Panel2.ResumeLayout(false);
			((System.ComponentModel.ISupportInitialize)splitContainer2).EndInit();
			splitContainer2.ResumeLayout(false);
			splitContainer3.Panel1.ResumeLayout(false);
			splitContainer3.Panel2.ResumeLayout(false);
			((System.ComponentModel.ISupportInitialize)splitContainer3).EndInit();
			splitContainer3.ResumeLayout(false);
			splitContainer4.Panel1.ResumeLayout(false);
			splitContainer4.Panel2.ResumeLayout(false);
			((System.ComponentModel.ISupportInitialize)splitContainer4).EndInit();
			splitContainer4.ResumeLayout(false);
			tabControl2.ResumeLayout(false);
			tabPage3.ResumeLayout(false);
			tabPage4.ResumeLayout(false);
			tabPage4.PerformLayout();
			toolStrip2.ResumeLayout(false);
			toolStrip2.PerformLayout();
			ResumeLayout(false);
			PerformLayout();
		}

		#endregion

		private MenuStrip menuStrip1;
		private ToolStripMenuItem fileToolStripMenuItem;
		private ToolStripMenuItem helpToolStripMenuItem;
		private ToolStripMenuItem aboutToolStripMenuItem;
		private StatusStrip statusStrip1;
		private ToolStripStatusLabel toolStripStatusLabel1;
		private ToolStripSeparator toolStripSeparator1;
		private ToolStripMenuItem exitToolStripMenuItem;
		private ToolStripMenuItem settingsToolStripMenuItem;
		private ToolStripMenuItem controlsToolStripMenuItem;
		private ToolStripMenuItem playToolStripMenuItem;
		private ToolStripMenuItem pauseToolStripMenuItem;
		private ToolStripMenuItem stopToolStripMenuItem;
		private OpenFileDialog openFileDialogNSF;
		private ToolStripMenuItem debugToolStripMenuItem;
		private OpenFileDialog openFileDialogHEX;
		private SaveFileDialog saveFileDialogBin;
		private ToolStrip toolStrip1;
		private ToolStripButton toolStripButtonPlay;
		private ToolStripButton toolStripButtonDiscard;
		private ToolStripButton toolStripButtonStop;
		private TabControl tabControl1;
		private TabPage tabPage1;
		private ToolStripStatusLabel toolStripStatusLabelSamples;
		private ToolStripStatusLabel toolStripStatusLabel3;
		private ToolStripStatusLabel toolStripStatusLabelMsec;
		private ToolStripStatusLabel toolStripStatusLabel5;
		private ToolStripStatusLabel toolStripStatusLabelState;
		private ToolStripStatusLabel toolStripStatusLabel7;
		private ToolStripStatusLabel toolStripStatusLabelACLK;
		private SplitContainer splitContainer1;
		private SignalPlot signalPlot1;
		private FurryPlot furryPlot1;
		private TabPage tabPage2;
		private SplitContainer splitContainer2;
		private SplitContainer splitContainer3;
		private Button button2;
		private ComboBox comboBox2;
		private PropertyGrid propertyGrid1;
		private SplitContainer splitContainer4;
		private Button button1;
		private ComboBox comboBox1;
		private ToolStripMenuItem sendFeedbackToolStripMenuItem;
		private ToolStripMenuItem checkForUpdatesToolStripMenuItem;
		private ToolStripSeparator toolStripSeparator3;
		private System.ComponentModel.BackgroundWorker backgroundWorker1;
		private ToolStripSeparator toolStripSeparator4;
		private ToolStripButton toolStripButton1;
		private Button button3;
		private OpenFileDialog openFileDialogRegDump;
		private ToolStripMenuItem loadAPURegisterDumpToolStripMenuItem;
		private ToolStripMenuItem loadAUXDumpToolStripMenuItem;
		private ToolStripMenuItem loadSaveLogisimAUXDumpForAudacityToolStripMenuItem;
		private ToolStripMenuItem saveSampleBufferAsWAVToolStripMenuItem;
		private SaveFileDialog saveFileDialogWAV;
		private ToolStripButton toolStripButton2;
		private ToolStripMenuItem loadCPUMemoryDumpToolStripMenuItem;
		private OpenFileDialog openFileDialogCpumem;
		private TabControl tabControl2;
		private TabPage tabPage3;
		private Be.Windows.Forms.HexBox hexBox1;
		private TabPage tabPage4;
		private WavesControl wavesControl1;
		private ToolStrip toolStrip2;
		private ToolStripButton toolStripButton3;
		private ToolStripButton toolStripButton4;
		private ToolStripSeparator toolStripSeparator5;
		private ToolStripButton toolStripButton5;
		private ToolStripButton toolStripButton6;
		private ToolStripButton toolStripButton7;
		private ToolStripButton toolStripButton8;
		private ToolStripButton toolStripButton9;
		private ToolStripButton toolStripButton10;
		private ToolStripStatusLabel toolStripStatusLabel6;
		private ToolStripStatusLabel toolStripStatusLabelACLKCount;
	}
}