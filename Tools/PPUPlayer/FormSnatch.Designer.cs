using System.Windows.Forms;

namespace PPUPlayer
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
			toolStrip1 = new ToolStrip();
			toolStripButton1 = new ToolStripButton();
			signalPlot1 = new SignalPlot();
			toolStrip1.SuspendLayout();
			SuspendLayout();
			// 
			// toolStrip1
			// 
			toolStrip1.Items.AddRange(new ToolStripItem[] { toolStripButton1 });
			toolStrip1.Location = new System.Drawing.Point(0, 0);
			toolStrip1.Name = "toolStrip1";
			toolStrip1.Size = new System.Drawing.Size(990, 25);
			toolStrip1.TabIndex = 0;
			toolStrip1.Text = "toolStrip1";
			// 
			// toolStripButton1
			// 
			toolStripButton1.Alignment = ToolStripItemAlignment.Right;
			toolStripButton1.DisplayStyle = ToolStripItemDisplayStyle.Text;
			toolStripButton1.Image = (System.Drawing.Image)resources.GetObject("toolStripButton1.Image");
			toolStripButton1.ImageTransparentColor = System.Drawing.Color.Magenta;
			toolStripButton1.Name = "toolStripButton1";
			toolStripButton1.Size = new System.Drawing.Size(47, 22);
			toolStripButton1.Text = "Snatch";
			toolStripButton1.Click += toolStripButton1_Click;
			// 
			// signalPlot1
			// 
			signalPlot1.Dock = DockStyle.Fill;
			signalPlot1.DottedColor = System.Drawing.Color.Silver;
			signalPlot1.DottedOpacity = 75;
			signalPlot1.FillColor = System.Drawing.Color.Black;
			signalPlot1.GridColor = System.Drawing.Color.Green;
			signalPlot1.GridOpacity = 95;
			signalPlot1.LabelsColor = System.Drawing.Color.White;
			signalPlot1.Location = new System.Drawing.Point(0, 25);
			signalPlot1.Name = "signalPlot1";
			signalPlot1.SelectionColor = System.Drawing.Color.GhostWhite;
			signalPlot1.SignalColor = System.Drawing.Color.SpringGreen;
			signalPlot1.Size = new System.Drawing.Size(990, 407);
			signalPlot1.TabIndex = 1;
			signalPlot1.Text = "signalPlot1";
			signalPlot1.ZeroColor = System.Drawing.Color.Red;
			// 
			// FormSnatch
			// 
			AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
			AutoScaleMode = AutoScaleMode.Font;
			ClientSize = new System.Drawing.Size(990, 432);
			Controls.Add(signalPlot1);
			Controls.Add(toolStrip1);
			KeyPreview = true;
			Name = "FormSnatch";
			StartPosition = FormStartPosition.CenterParent;
			Text = "Snatch Wave";
			KeyDown += FormSnatch_KeyDown;
			toolStrip1.ResumeLayout(false);
			toolStrip1.PerformLayout();
			ResumeLayout(false);
			PerformLayout();
		}

		#endregion

		private ToolStrip toolStrip1;
		private ToolStripButton toolStripButton1;
		private SignalPlot signalPlot1;
	}
}