namespace SlooowPlaaayer
{
	partial class FormSnatch
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
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(FormSnatch));
			this.toolStrip1 = new System.Windows.Forms.ToolStrip();
			this.toolStripButton1 = new System.Windows.Forms.ToolStripButton();
			this.signalPlot1 = new System.Windows.Forms.SignalPlot();
			this.toolStrip1.SuspendLayout();
			this.SuspendLayout();
			// 
			// toolStrip1
			// 
			this.toolStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripButton1});
			this.toolStrip1.Location = new System.Drawing.Point(0, 0);
			this.toolStrip1.Name = "toolStrip1";
			this.toolStrip1.Size = new System.Drawing.Size(990, 25);
			this.toolStrip1.TabIndex = 0;
			this.toolStrip1.Text = "toolStrip1";
			// 
			// toolStripButton1
			// 
			this.toolStripButton1.Alignment = System.Windows.Forms.ToolStripItemAlignment.Right;
			this.toolStripButton1.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Text;
			this.toolStripButton1.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButton1.Image")));
			this.toolStripButton1.ImageTransparentColor = System.Drawing.Color.Magenta;
			this.toolStripButton1.Name = "toolStripButton1";
			this.toolStripButton1.Size = new System.Drawing.Size(47, 22);
			this.toolStripButton1.Text = "Snatch";
			this.toolStripButton1.Click += new System.EventHandler(this.toolStripButton1_Click);
			// 
			// signalPlot1
			// 
			this.signalPlot1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.signalPlot1.FillColor = System.Drawing.Color.Empty;
			this.signalPlot1.GridColor = System.Drawing.Color.Green;
			this.signalPlot1.LabelsColor = System.Drawing.Color.White;
			this.signalPlot1.Location = new System.Drawing.Point(0, 25);
			this.signalPlot1.Name = "signalPlot1";
			this.signalPlot1.SelectionColor = System.Drawing.Color.GhostWhite;
			this.signalPlot1.SignalColor = System.Drawing.Color.SpringGreen;
			this.signalPlot1.Size = new System.Drawing.Size(990, 407);
			this.signalPlot1.TabIndex = 1;
			this.signalPlot1.Text = "signalPlot1";
			this.signalPlot1.ZeroColor = System.Drawing.Color.Red;
			// 
			// FormSnatch
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(990, 432);
			this.Controls.Add(this.signalPlot1);
			this.Controls.Add(this.toolStrip1);
			this.KeyPreview = true;
			this.Name = "FormSnatch";
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			this.Text = "Snatch Wave";
			this.KeyDown += new System.Windows.Forms.KeyEventHandler(this.FormSnatch_KeyDown);
			this.toolStrip1.ResumeLayout(false);
			this.toolStrip1.PerformLayout();
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private ToolStrip toolStrip1;
		private ToolStripButton toolStripButton1;
		private SignalPlot signalPlot1;
	}
}