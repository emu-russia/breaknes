namespace NSFPlayer
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
			this.menuStrip1 = new System.Windows.Forms.MenuStrip();
			this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.loadNSFToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.settingsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
			this.exitToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.debugToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.loadSaveLogisimHexAsBinToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.controlsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.playToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.pauseToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.stopToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripSeparator2 = new System.Windows.Forms.ToolStripSeparator();
			this.previousTrackToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.nextTrackToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.helpToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.nSFInfoToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.aboutToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.statusStrip1 = new System.Windows.Forms.StatusStrip();
			this.toolStripStatusLabel1 = new System.Windows.Forms.ToolStripStatusLabel();
			this.toolStripStatusLabelSamples = new System.Windows.Forms.ToolStripStatusLabel();
			this.toolStripStatusLabel3 = new System.Windows.Forms.ToolStripStatusLabel();
			this.toolStripStatusLabelMsec = new System.Windows.Forms.ToolStripStatusLabel();
			this.toolStripStatusLabel5 = new System.Windows.Forms.ToolStripStatusLabel();
			this.toolStripStatusLabelState = new System.Windows.Forms.ToolStripStatusLabel();
			this.toolStripStatusLabel2 = new System.Windows.Forms.ToolStripStatusLabel();
			this.toolStripStatusLabel4 = new System.Windows.Forms.ToolStripStatusLabel();
			this.toolStripStatusLabel7 = new System.Windows.Forms.ToolStripStatusLabel();
			this.toolStripStatusLabelACLK = new System.Windows.Forms.ToolStripStatusLabel();
			this.openFileDialog1 = new System.Windows.Forms.OpenFileDialog();
			this.openFileDialogHEX = new System.Windows.Forms.OpenFileDialog();
			this.saveFileDialogBin = new System.Windows.Forms.SaveFileDialog();
			this.toolStrip1 = new System.Windows.Forms.ToolStrip();
			this.toolStripButtonPlay = new System.Windows.Forms.ToolStripButton();
			this.toolStripButtonDiscard = new System.Windows.Forms.ToolStripButton();
			this.toolStripButtonStop = new System.Windows.Forms.ToolStripButton();
			this.tabControl1 = new System.Windows.Forms.TabControl();
			this.tabPage1 = new System.Windows.Forms.TabPage();
			this.splitContainer1 = new System.Windows.Forms.SplitContainer();
			this.signalPlot1 = new System.Windows.Forms.SignalPlot();
			this.furryPlot1 = new System.Windows.Forms.FurryPlot();
			this.tabPage2 = new System.Windows.Forms.TabPage();
			this.menuStrip1.SuspendLayout();
			this.statusStrip1.SuspendLayout();
			this.toolStrip1.SuspendLayout();
			this.tabControl1.SuspendLayout();
			this.tabPage1.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.splitContainer1)).BeginInit();
			this.splitContainer1.Panel1.SuspendLayout();
			this.splitContainer1.Panel2.SuspendLayout();
			this.splitContainer1.SuspendLayout();
			this.SuspendLayout();
			// 
			// menuStrip1
			// 
			this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem,
            this.debugToolStripMenuItem,
            this.controlsToolStripMenuItem,
            this.helpToolStripMenuItem});
			this.menuStrip1.Location = new System.Drawing.Point(0, 0);
			this.menuStrip1.Name = "menuStrip1";
			this.menuStrip1.Size = new System.Drawing.Size(1018, 24);
			this.menuStrip1.TabIndex = 0;
			this.menuStrip1.Text = "menuStrip1";
			// 
			// fileToolStripMenuItem
			// 
			this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.loadNSFToolStripMenuItem,
            this.settingsToolStripMenuItem,
            this.toolStripSeparator1,
            this.exitToolStripMenuItem});
			this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
			this.fileToolStripMenuItem.Size = new System.Drawing.Size(37, 20);
			this.fileToolStripMenuItem.Text = "File";
			// 
			// loadNSFToolStripMenuItem
			// 
			this.loadNSFToolStripMenuItem.Name = "loadNSFToolStripMenuItem";
			this.loadNSFToolStripMenuItem.Size = new System.Drawing.Size(133, 22);
			this.loadNSFToolStripMenuItem.Text = "Load NSF...";
			this.loadNSFToolStripMenuItem.Click += new System.EventHandler(this.loadNSFToolStripMenuItem_Click);
			// 
			// settingsToolStripMenuItem
			// 
			this.settingsToolStripMenuItem.Name = "settingsToolStripMenuItem";
			this.settingsToolStripMenuItem.Size = new System.Drawing.Size(133, 22);
			this.settingsToolStripMenuItem.Text = "Settings...";
			this.settingsToolStripMenuItem.Click += new System.EventHandler(this.settingsToolStripMenuItem_Click);
			// 
			// toolStripSeparator1
			// 
			this.toolStripSeparator1.Name = "toolStripSeparator1";
			this.toolStripSeparator1.Size = new System.Drawing.Size(130, 6);
			// 
			// exitToolStripMenuItem
			// 
			this.exitToolStripMenuItem.Name = "exitToolStripMenuItem";
			this.exitToolStripMenuItem.Size = new System.Drawing.Size(133, 22);
			this.exitToolStripMenuItem.Text = "Exit";
			this.exitToolStripMenuItem.Click += new System.EventHandler(this.exitToolStripMenuItem_Click);
			// 
			// debugToolStripMenuItem
			// 
			this.debugToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.loadSaveLogisimHexAsBinToolStripMenuItem});
			this.debugToolStripMenuItem.Name = "debugToolStripMenuItem";
			this.debugToolStripMenuItem.Size = new System.Drawing.Size(54, 20);
			this.debugToolStripMenuItem.Text = "Debug";
			// 
			// loadSaveLogisimHexAsBinToolStripMenuItem
			// 
			this.loadSaveLogisimHexAsBinToolStripMenuItem.Name = "loadSaveLogisimHexAsBinToolStripMenuItem";
			this.loadSaveLogisimHexAsBinToolStripMenuItem.Size = new System.Drawing.Size(238, 22);
			this.loadSaveLogisimHexAsBinToolStripMenuItem.Text = "Load->Save Logisim hex as bin";
			this.loadSaveLogisimHexAsBinToolStripMenuItem.Click += new System.EventHandler(this.loadSaveLogisimHexAsBinToolStripMenuItem_Click);
			// 
			// controlsToolStripMenuItem
			// 
			this.controlsToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.playToolStripMenuItem,
            this.pauseToolStripMenuItem,
            this.stopToolStripMenuItem,
            this.toolStripSeparator2,
            this.previousTrackToolStripMenuItem,
            this.nextTrackToolStripMenuItem});
			this.controlsToolStripMenuItem.Name = "controlsToolStripMenuItem";
			this.controlsToolStripMenuItem.Size = new System.Drawing.Size(88, 20);
			this.controlsToolStripMenuItem.Text = "NSF Controls";
			// 
			// playToolStripMenuItem
			// 
			this.playToolStripMenuItem.Name = "playToolStripMenuItem";
			this.playToolStripMenuItem.Size = new System.Drawing.Size(149, 22);
			this.playToolStripMenuItem.Text = "Play";
			this.playToolStripMenuItem.Click += new System.EventHandler(this.playToolStripMenuItem_Click);
			// 
			// pauseToolStripMenuItem
			// 
			this.pauseToolStripMenuItem.Name = "pauseToolStripMenuItem";
			this.pauseToolStripMenuItem.Size = new System.Drawing.Size(149, 22);
			this.pauseToolStripMenuItem.Text = "Pause";
			this.pauseToolStripMenuItem.Click += new System.EventHandler(this.pauseToolStripMenuItem_Click);
			// 
			// stopToolStripMenuItem
			// 
			this.stopToolStripMenuItem.Name = "stopToolStripMenuItem";
			this.stopToolStripMenuItem.Size = new System.Drawing.Size(149, 22);
			this.stopToolStripMenuItem.Text = "Stop";
			this.stopToolStripMenuItem.Click += new System.EventHandler(this.stopToolStripMenuItem_Click);
			// 
			// toolStripSeparator2
			// 
			this.toolStripSeparator2.Name = "toolStripSeparator2";
			this.toolStripSeparator2.Size = new System.Drawing.Size(146, 6);
			// 
			// previousTrackToolStripMenuItem
			// 
			this.previousTrackToolStripMenuItem.Name = "previousTrackToolStripMenuItem";
			this.previousTrackToolStripMenuItem.Size = new System.Drawing.Size(149, 22);
			this.previousTrackToolStripMenuItem.Text = "Previous Track";
			this.previousTrackToolStripMenuItem.Click += new System.EventHandler(this.previousTrackToolStripMenuItem_Click);
			// 
			// nextTrackToolStripMenuItem
			// 
			this.nextTrackToolStripMenuItem.Name = "nextTrackToolStripMenuItem";
			this.nextTrackToolStripMenuItem.Size = new System.Drawing.Size(149, 22);
			this.nextTrackToolStripMenuItem.Text = "Next Track";
			this.nextTrackToolStripMenuItem.Click += new System.EventHandler(this.nextTrackToolStripMenuItem_Click);
			// 
			// helpToolStripMenuItem
			// 
			this.helpToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.nSFInfoToolStripMenuItem,
            this.aboutToolStripMenuItem});
			this.helpToolStripMenuItem.Name = "helpToolStripMenuItem";
			this.helpToolStripMenuItem.Size = new System.Drawing.Size(44, 20);
			this.helpToolStripMenuItem.Text = "Help";
			// 
			// nSFInfoToolStripMenuItem
			// 
			this.nSFInfoToolStripMenuItem.Name = "nSFInfoToolStripMenuItem";
			this.nSFInfoToolStripMenuItem.Size = new System.Drawing.Size(119, 22);
			this.nSFInfoToolStripMenuItem.Text = "NSF Info";
			this.nSFInfoToolStripMenuItem.Click += new System.EventHandler(this.nSFInfoToolStripMenuItem_Click);
			// 
			// aboutToolStripMenuItem
			// 
			this.aboutToolStripMenuItem.Name = "aboutToolStripMenuItem";
			this.aboutToolStripMenuItem.Size = new System.Drawing.Size(119, 22);
			this.aboutToolStripMenuItem.Text = "About";
			this.aboutToolStripMenuItem.Click += new System.EventHandler(this.aboutToolStripMenuItem_Click);
			// 
			// statusStrip1
			// 
			this.statusStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripStatusLabel1,
            this.toolStripStatusLabelSamples,
            this.toolStripStatusLabel3,
            this.toolStripStatusLabelMsec,
            this.toolStripStatusLabel5,
            this.toolStripStatusLabelState,
            this.toolStripStatusLabel2,
            this.toolStripStatusLabel4,
            this.toolStripStatusLabel7,
            this.toolStripStatusLabelACLK});
			this.statusStrip1.Location = new System.Drawing.Point(0, 533);
			this.statusStrip1.Name = "statusStrip1";
			this.statusStrip1.Size = new System.Drawing.Size(1018, 24);
			this.statusStrip1.TabIndex = 1;
			this.statusStrip1.Text = "statusStrip1";
			// 
			// toolStripStatusLabel1
			// 
			this.toolStripStatusLabel1.Name = "toolStripStatusLabel1";
			this.toolStripStatusLabel1.Size = new System.Drawing.Size(105, 19);
			this.toolStripStatusLabel1.Text = "Samples collected:";
			// 
			// toolStripStatusLabelSamples
			// 
			this.toolStripStatusLabelSamples.BorderSides = System.Windows.Forms.ToolStripStatusLabelBorderSides.Right;
			this.toolStripStatusLabelSamples.Name = "toolStripStatusLabelSamples";
			this.toolStripStatusLabelSamples.Size = new System.Drawing.Size(17, 19);
			this.toolStripStatusLabelSamples.Text = "0";
			// 
			// toolStripStatusLabel3
			// 
			this.toolStripStatusLabel3.Name = "toolStripStatusLabel3";
			this.toolStripStatusLabel3.Size = new System.Drawing.Size(91, 19);
			this.toolStripStatusLabel3.Text = "Samples length:";
			// 
			// toolStripStatusLabelMsec
			// 
			this.toolStripStatusLabelMsec.BorderSides = System.Windows.Forms.ToolStripStatusLabelBorderSides.Right;
			this.toolStripStatusLabelMsec.Name = "toolStripStatusLabelMsec";
			this.toolStripStatusLabelMsec.Size = new System.Drawing.Size(36, 19);
			this.toolStripStatusLabelMsec.Text = "0 ms";
			// 
			// toolStripStatusLabel5
			// 
			this.toolStripStatusLabel5.Name = "toolStripStatusLabel5";
			this.toolStripStatusLabel5.Size = new System.Drawing.Size(48, 19);
			this.toolStripStatusLabel5.Text = "Worker:";
			// 
			// toolStripStatusLabelState
			// 
			this.toolStripStatusLabelState.BorderSides = System.Windows.Forms.ToolStripStatusLabelBorderSides.Right;
			this.toolStripStatusLabelState.Name = "toolStripStatusLabelState";
			this.toolStripStatusLabelState.Size = new System.Drawing.Size(49, 19);
			this.toolStripStatusLabelState.Text = "Paused";
			// 
			// toolStripStatusLabel2
			// 
			this.toolStripStatusLabel2.Name = "toolStripStatusLabel2";
			this.toolStripStatusLabel2.Size = new System.Drawing.Size(60, 19);
			this.toolStripStatusLabel2.Text = "NSF track:";
			// 
			// toolStripStatusLabel4
			// 
			this.toolStripStatusLabel4.BorderSides = System.Windows.Forms.ToolStripStatusLabelBorderSides.Right;
			this.toolStripStatusLabel4.Name = "toolStripStatusLabel4";
			this.toolStripStatusLabel4.Size = new System.Drawing.Size(70, 19);
			this.toolStripStatusLabel4.Text = "Not loaded";
			// 
			// toolStripStatusLabel7
			// 
			this.toolStripStatusLabel7.Name = "toolStripStatusLabel7";
			this.toolStripStatusLabel7.Size = new System.Drawing.Size(62, 19);
			this.toolStripStatusLabel7.Text = "ACLK/Sec:";
			// 
			// toolStripStatusLabelACLK
			// 
			this.toolStripStatusLabelACLK.Name = "toolStripStatusLabelACLK";
			this.toolStripStatusLabelACLK.Size = new System.Drawing.Size(13, 19);
			this.toolStripStatusLabelACLK.Text = "0";
			// 
			// openFileDialog1
			// 
			this.openFileDialog1.DefaultExt = "nsf";
			this.openFileDialog1.Filter = "NSF files|*.nsf|All files|*.*";
			// 
			// openFileDialogHEX
			// 
			this.openFileDialogHEX.DefaultExt = "hex";
			this.openFileDialogHEX.Filter = "Logisim HEX files|*.hex|All files|*.*";
			// 
			// saveFileDialogBin
			// 
			this.saveFileDialogBin.DefaultExt = "bin";
			this.saveFileDialogBin.Filter = "Binary files|*.bin|All files|*.*";
			// 
			// toolStrip1
			// 
			this.toolStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripButtonPlay,
            this.toolStripButtonDiscard,
            this.toolStripButtonStop});
			this.toolStrip1.Location = new System.Drawing.Point(0, 24);
			this.toolStrip1.Name = "toolStrip1";
			this.toolStrip1.Size = new System.Drawing.Size(1018, 25);
			this.toolStrip1.TabIndex = 2;
			this.toolStrip1.Text = "toolStrip1";
			// 
			// toolStripButtonPlay
			// 
			this.toolStripButtonPlay.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Text;
			this.toolStripButtonPlay.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonPlay.Image")));
			this.toolStripButtonPlay.ImageTransparentColor = System.Drawing.Color.Magenta;
			this.toolStripButtonPlay.Name = "toolStripButtonPlay";
			this.toolStripButtonPlay.Size = new System.Drawing.Size(33, 22);
			this.toolStripButtonPlay.Text = "Play";
			this.toolStripButtonPlay.Click += new System.EventHandler(this.toolStripButtonPlay_Click);
			// 
			// toolStripButtonDiscard
			// 
			this.toolStripButtonDiscard.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Text;
			this.toolStripButtonDiscard.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonDiscard.Image")));
			this.toolStripButtonDiscard.ImageTransparentColor = System.Drawing.Color.Magenta;
			this.toolStripButtonDiscard.Name = "toolStripButtonDiscard";
			this.toolStripButtonDiscard.Size = new System.Drawing.Size(50, 22);
			this.toolStripButtonDiscard.Text = "Discard";
			this.toolStripButtonDiscard.Click += new System.EventHandler(this.toolStripButtonDiscard_Click);
			// 
			// toolStripButtonStop
			// 
			this.toolStripButtonStop.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Text;
			this.toolStripButtonStop.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonStop.Image")));
			this.toolStripButtonStop.ImageTransparentColor = System.Drawing.Color.Magenta;
			this.toolStripButtonStop.Name = "toolStripButtonStop";
			this.toolStripButtonStop.Size = new System.Drawing.Size(35, 22);
			this.toolStripButtonStop.Text = "Stop";
			this.toolStripButtonStop.Click += new System.EventHandler(this.toolStripButtonStop_Click);
			// 
			// tabControl1
			// 
			this.tabControl1.Controls.Add(this.tabPage1);
			this.tabControl1.Controls.Add(this.tabPage2);
			this.tabControl1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tabControl1.Location = new System.Drawing.Point(0, 49);
			this.tabControl1.Name = "tabControl1";
			this.tabControl1.SelectedIndex = 0;
			this.tabControl1.Size = new System.Drawing.Size(1018, 484);
			this.tabControl1.TabIndex = 3;
			// 
			// tabPage1
			// 
			this.tabPage1.Controls.Add(this.splitContainer1);
			this.tabPage1.Location = new System.Drawing.Point(4, 24);
			this.tabPage1.Name = "tabPage1";
			this.tabPage1.Padding = new System.Windows.Forms.Padding(3);
			this.tabPage1.Size = new System.Drawing.Size(1010, 456);
			this.tabPage1.TabIndex = 0;
			this.tabPage1.Text = "Wave Dump";
			this.tabPage1.UseVisualStyleBackColor = true;
			// 
			// splitContainer1
			// 
			this.splitContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.splitContainer1.Location = new System.Drawing.Point(3, 3);
			this.splitContainer1.Name = "splitContainer1";
			this.splitContainer1.Orientation = System.Windows.Forms.Orientation.Horizontal;
			// 
			// splitContainer1.Panel1
			// 
			this.splitContainer1.Panel1.Controls.Add(this.signalPlot1);
			// 
			// splitContainer1.Panel2
			// 
			this.splitContainer1.Panel2.Controls.Add(this.furryPlot1);
			this.splitContainer1.Size = new System.Drawing.Size(1004, 450);
			this.splitContainer1.SplitterDistance = 261;
			this.splitContainer1.TabIndex = 0;
			// 
			// signalPlot1
			// 
			this.signalPlot1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.signalPlot1.FillColor = System.Drawing.Color.Black;
			this.signalPlot1.Font = new System.Drawing.Font("Segoe UI", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
			this.signalPlot1.GridColor = System.Drawing.Color.Green;
			this.signalPlot1.LabelsColor = System.Drawing.Color.White;
			this.signalPlot1.Location = new System.Drawing.Point(0, 0);
			this.signalPlot1.Name = "signalPlot1";
			this.signalPlot1.SignalColor = System.Drawing.Color.LightGreen;
			this.signalPlot1.Size = new System.Drawing.Size(1004, 261);
			this.signalPlot1.TabIndex = 0;
			this.signalPlot1.Text = "signalPlot1";
			this.signalPlot1.ZeroColor = System.Drawing.Color.Red;
			// 
			// furryPlot1
			// 
			this.furryPlot1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.furryPlot1.Location = new System.Drawing.Point(0, 0);
			this.furryPlot1.Name = "furryPlot1";
			this.furryPlot1.Size = new System.Drawing.Size(1004, 185);
			this.furryPlot1.TabIndex = 0;
			this.furryPlot1.Text = "furryPlot1";
			// 
			// tabPage2
			// 
			this.tabPage2.Location = new System.Drawing.Point(4, 24);
			this.tabPage2.Name = "tabPage2";
			this.tabPage2.Padding = new System.Windows.Forms.Padding(3);
			this.tabPage2.Size = new System.Drawing.Size(1010, 456);
			this.tabPage2.TabIndex = 1;
			this.tabPage2.Text = "APU Debug";
			this.tabPage2.UseVisualStyleBackColor = true;
			// 
			// FormMain
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(1018, 557);
			this.Controls.Add(this.tabControl1);
			this.Controls.Add(this.toolStrip1);
			this.Controls.Add(this.statusStrip1);
			this.Controls.Add(this.menuStrip1);
			this.MainMenuStrip = this.menuStrip1;
			this.Name = "FormMain";
			this.Text = "NSF Player";
			this.Load += new System.EventHandler(this.FormMain_Load);
			this.menuStrip1.ResumeLayout(false);
			this.menuStrip1.PerformLayout();
			this.statusStrip1.ResumeLayout(false);
			this.statusStrip1.PerformLayout();
			this.toolStrip1.ResumeLayout(false);
			this.toolStrip1.PerformLayout();
			this.tabControl1.ResumeLayout(false);
			this.tabPage1.ResumeLayout(false);
			this.splitContainer1.Panel1.ResumeLayout(false);
			this.splitContainer1.Panel2.ResumeLayout(false);
			((System.ComponentModel.ISupportInitialize)(this.splitContainer1)).EndInit();
			this.splitContainer1.ResumeLayout(false);
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private MenuStrip menuStrip1;
		private ToolStripMenuItem fileToolStripMenuItem;
		private ToolStripMenuItem helpToolStripMenuItem;
		private ToolStripMenuItem aboutToolStripMenuItem;
		private StatusStrip statusStrip1;
		private ToolStripStatusLabel toolStripStatusLabel1;
		private ToolStripMenuItem loadNSFToolStripMenuItem;
		private ToolStripSeparator toolStripSeparator1;
		private ToolStripMenuItem exitToolStripMenuItem;
		private ToolStripMenuItem settingsToolStripMenuItem;
		private ToolStripMenuItem controlsToolStripMenuItem;
		private ToolStripMenuItem playToolStripMenuItem;
		private ToolStripMenuItem pauseToolStripMenuItem;
		private ToolStripMenuItem stopToolStripMenuItem;
		private ToolStripSeparator toolStripSeparator2;
		private ToolStripMenuItem previousTrackToolStripMenuItem;
		private ToolStripMenuItem nextTrackToolStripMenuItem;
		private ToolStripMenuItem nSFInfoToolStripMenuItem;
		private OpenFileDialog openFileDialog1;
		private ToolStripMenuItem debugToolStripMenuItem;
		private ToolStripMenuItem loadSaveLogisimHexAsBinToolStripMenuItem;
		private OpenFileDialog openFileDialogHEX;
		private SaveFileDialog saveFileDialogBin;
		private ToolStrip toolStrip1;
		private ToolStripButton toolStripButtonPlay;
		private ToolStripButton toolStripButtonDiscard;
		private ToolStripButton toolStripButtonStop;
		private TabControl tabControl1;
		private TabPage tabPage1;
		private TabPage tabPage2;
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
		private ToolStripStatusLabel toolStripStatusLabel2;
		private ToolStripStatusLabel toolStripStatusLabel4;
	}
}