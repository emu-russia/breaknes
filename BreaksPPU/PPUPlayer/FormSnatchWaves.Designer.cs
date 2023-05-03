namespace PPUPlayer
{
	partial class FormSnatchWaves
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
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(FormSnatchWaves));
			toolStrip1 = new System.Windows.Forms.ToolStrip();
			toolStripButton1 = new System.Windows.Forms.ToolStripButton();
			wavesControl1 = new System.Windows.Forms.WavesControl();
			toolStrip1.SuspendLayout();
			SuspendLayout();
			// 
			// toolStrip1
			// 
			toolStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] { toolStripButton1 });
			toolStrip1.Location = new System.Drawing.Point(0, 0);
			toolStrip1.Name = "toolStrip1";
			toolStrip1.Size = new System.Drawing.Size(1025, 25);
			toolStrip1.TabIndex = 0;
			toolStrip1.Text = "toolStrip1";
			// 
			// toolStripButton1
			// 
			toolStripButton1.Alignment = System.Windows.Forms.ToolStripItemAlignment.Right;
			toolStripButton1.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Text;
			toolStripButton1.Image = (System.Drawing.Image)resources.GetObject("toolStripButton1.Image");
			toolStripButton1.ImageTransparentColor = System.Drawing.Color.Magenta;
			toolStripButton1.Name = "toolStripButton1";
			toolStripButton1.Size = new System.Drawing.Size(47, 22);
			toolStripButton1.Text = "Snatch";
			toolStripButton1.Click += toolStripButton1_Click;
			// 
			// wavesControl1
			// 
			wavesControl1.Dock = System.Windows.Forms.DockStyle.Fill;
			wavesControl1.DottedColor = System.Drawing.Color.Empty;
			wavesControl1.DottedOpacity = 0;
			wavesControl1.FillColor = System.Drawing.Color.Empty;
			wavesControl1.GridColor = System.Drawing.Color.Empty;
			wavesControl1.GridOpacity = 0;
			wavesControl1.HighZColor = System.Drawing.Color.Empty;
			wavesControl1.LabelsColor = System.Drawing.Color.Empty;
			wavesControl1.Location = new System.Drawing.Point(0, 25);
			wavesControl1.Name = "wavesControl1";
			wavesControl1.SelectionColor = System.Drawing.Color.Empty;
			wavesControl1.SignalColor = System.Drawing.Color.Empty;
			wavesControl1.Size = new System.Drawing.Size(1025, 526);
			wavesControl1.TabIndex = 1;
			wavesControl1.Text = "wavesControl1";
			wavesControl1.UndefinedColor = System.Drawing.Color.Empty;
			// 
			// FormSnatchWaves
			// 
			AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
			AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			ClientSize = new System.Drawing.Size(1025, 551);
			Controls.Add(wavesControl1);
			Controls.Add(toolStrip1);
			KeyPreview = true;
			Name = "FormSnatchWaves";
			Text = "Waves";
			KeyUp += FormSnatchWaves_KeyUp;
			toolStrip1.ResumeLayout(false);
			toolStrip1.PerformLayout();
			ResumeLayout(false);
			PerformLayout();
		}

		#endregion

		private System.Windows.Forms.ToolStrip toolStrip1;
		private System.Windows.Forms.ToolStripButton toolStripButton1;
		private System.Windows.Forms.WavesControl wavesControl1;
	}
}