namespace SignalPlotDemo
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
			this.menuStrip1 = new System.Windows.Forms.MenuStrip();
			this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.exitToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.debugToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.normalizedNoiseToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.normalizedSineWaveToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
			this.loadFloatsDumpToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.saveFloatsDumpToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.openFileDialog1 = new System.Windows.Forms.OpenFileDialog();
			this.saveFileDialog1 = new System.Windows.Forms.SaveFileDialog();
			this.signalPlot1 = new System.Windows.Forms.SignalPlot();
			this.menuStrip1.SuspendLayout();
			this.SuspendLayout();
			// 
			// menuStrip1
			// 
			this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem,
            this.debugToolStripMenuItem});
			this.menuStrip1.Location = new System.Drawing.Point(0, 0);
			this.menuStrip1.Name = "menuStrip1";
			this.menuStrip1.Size = new System.Drawing.Size(593, 24);
			this.menuStrip1.TabIndex = 1;
			this.menuStrip1.Text = "menuStrip1";
			// 
			// fileToolStripMenuItem
			// 
			this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.exitToolStripMenuItem});
			this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
			this.fileToolStripMenuItem.Size = new System.Drawing.Size(37, 20);
			this.fileToolStripMenuItem.Text = "File";
			// 
			// exitToolStripMenuItem
			// 
			this.exitToolStripMenuItem.Name = "exitToolStripMenuItem";
			this.exitToolStripMenuItem.Size = new System.Drawing.Size(93, 22);
			this.exitToolStripMenuItem.Text = "Exit";
			this.exitToolStripMenuItem.Click += new System.EventHandler(this.exitToolStripMenuItem_Click);
			// 
			// debugToolStripMenuItem
			// 
			this.debugToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.normalizedNoiseToolStripMenuItem,
            this.normalizedSineWaveToolStripMenuItem,
            this.toolStripSeparator1,
            this.loadFloatsDumpToolStripMenuItem,
            this.saveFloatsDumpToolStripMenuItem});
			this.debugToolStripMenuItem.Name = "debugToolStripMenuItem";
			this.debugToolStripMenuItem.Size = new System.Drawing.Size(54, 20);
			this.debugToolStripMenuItem.Text = "Debug";
			// 
			// normalizedNoiseToolStripMenuItem
			// 
			this.normalizedNoiseToolStripMenuItem.Name = "normalizedNoiseToolStripMenuItem";
			this.normalizedNoiseToolStripMenuItem.Size = new System.Drawing.Size(176, 22);
			this.normalizedNoiseToolStripMenuItem.Text = "Normalized noise";
			this.normalizedNoiseToolStripMenuItem.Click += new System.EventHandler(this.normalizedNoiseToolStripMenuItem_Click);
			// 
			// normalizedSineWaveToolStripMenuItem
			// 
			this.normalizedSineWaveToolStripMenuItem.Name = "normalizedSineWaveToolStripMenuItem";
			this.normalizedSineWaveToolStripMenuItem.Size = new System.Drawing.Size(176, 22);
			this.normalizedSineWaveToolStripMenuItem.Text = "Sine wave";
			this.normalizedSineWaveToolStripMenuItem.Click += new System.EventHandler(this.normalizedSineWaveToolStripMenuItem_Click);
			// 
			// toolStripSeparator1
			// 
			this.toolStripSeparator1.Name = "toolStripSeparator1";
			this.toolStripSeparator1.Size = new System.Drawing.Size(173, 6);
			// 
			// loadFloatsDumpToolStripMenuItem
			// 
			this.loadFloatsDumpToolStripMenuItem.Name = "loadFloatsDumpToolStripMenuItem";
			this.loadFloatsDumpToolStripMenuItem.Size = new System.Drawing.Size(176, 22);
			this.loadFloatsDumpToolStripMenuItem.Text = "Load floats dump...";
			this.loadFloatsDumpToolStripMenuItem.Click += new System.EventHandler(this.loadFloatsDumpToolStripMenuItem_Click);
			// 
			// saveFloatsDumpToolStripMenuItem
			// 
			this.saveFloatsDumpToolStripMenuItem.Name = "saveFloatsDumpToolStripMenuItem";
			this.saveFloatsDumpToolStripMenuItem.Size = new System.Drawing.Size(176, 22);
			this.saveFloatsDumpToolStripMenuItem.Text = "Save floats dump...";
			this.saveFloatsDumpToolStripMenuItem.Click += new System.EventHandler(this.saveFloatsDumpToolStripMenuItem_Click);
			// 
			// openFileDialog1
			// 
			this.openFileDialog1.DefaultExt = "bin";
			this.openFileDialog1.Filter = "Binary files|*.bin|All files|*.*";
			// 
			// saveFileDialog1
			// 
			this.saveFileDialog1.DefaultExt = "bin";
			this.saveFileDialog1.Filter = "Binary files|*.bin|All files|*.*";
			// 
			// signalPlot1
			// 
			this.signalPlot1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.signalPlot1.DottedColor = System.Drawing.Color.Silver;
			this.signalPlot1.FillColor = System.Drawing.Color.Black;
			this.signalPlot1.Font = new System.Drawing.Font("Segoe UI", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
			this.signalPlot1.GridColor = System.Drawing.Color.Green;
			this.signalPlot1.GridOpacity = 95;
			this.signalPlot1.LabelsColor = System.Drawing.Color.White;
			this.signalPlot1.Location = new System.Drawing.Point(0, 24);
			this.signalPlot1.Name = "signalPlot1";
			this.signalPlot1.SelectionColor = System.Drawing.Color.GhostWhite;
			this.signalPlot1.SignalColor = System.Drawing.Color.LightGreen;
			this.signalPlot1.Size = new System.Drawing.Size(593, 284);
			this.signalPlot1.TabIndex = 2;
			this.signalPlot1.Text = "signalPlot1";
			this.signalPlot1.ZeroColor = System.Drawing.Color.Red;
			// 
			// FormMain
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(593, 308);
			this.Controls.Add(this.signalPlot1);
			this.Controls.Add(this.menuStrip1);
			this.MainMenuStrip = this.menuStrip1;
			this.MinimumSize = new System.Drawing.Size(100, 100);
			this.Name = "FormMain";
			this.Text = "Signal Plot Demo";
			this.menuStrip1.ResumeLayout(false);
			this.menuStrip1.PerformLayout();
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion
		private MenuStrip menuStrip1;
		private ToolStripMenuItem fileToolStripMenuItem;
		private ToolStripMenuItem exitToolStripMenuItem;
		private ToolStripMenuItem debugToolStripMenuItem;
		private ToolStripMenuItem normalizedNoiseToolStripMenuItem;
		private ToolStripMenuItem normalizedSineWaveToolStripMenuItem;
		private ToolStripSeparator toolStripSeparator1;
		private ToolStripMenuItem loadFloatsDumpToolStripMenuItem;
		private ToolStripMenuItem saveFloatsDumpToolStripMenuItem;
		private OpenFileDialog openFileDialog1;
		private SaveFileDialog saveFileDialog1;
		private SignalPlot signalPlot1;
	}
}