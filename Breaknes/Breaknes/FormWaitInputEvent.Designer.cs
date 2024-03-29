﻿namespace Breaknes
{
	partial class FormWaitInputEvent
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
			label1 = new Label();
			backgroundWorker1 = new System.ComponentModel.BackgroundWorker();
			SuspendLayout();
			// 
			// label1
			// 
			label1.AutoSize = true;
			label1.Font = new Font("Segoe UI", 18F, FontStyle.Bold | FontStyle.Italic, GraphicsUnit.Point);
			label1.ForeColor = SystemColors.Highlight;
			label1.Location = new Point(25, 21);
			label1.Name = "label1";
			label1.Size = new Size(492, 32);
			label1.TabIndex = 0;
			label1.Text = "Press a button or key on the input device...";
			// 
			// backgroundWorker1
			// 
			backgroundWorker1.WorkerSupportsCancellation = true;
			backgroundWorker1.DoWork += backgroundWorker1_DoWork;
			// 
			// FormWaitInputEvent
			// 
			AutoScaleDimensions = new SizeF(7F, 15F);
			AutoScaleMode = AutoScaleMode.Font;
			ClientSize = new Size(542, 81);
			Controls.Add(label1);
			KeyPreview = true;
			Name = "FormWaitInputEvent";
			StartPosition = FormStartPosition.CenterParent;
			Text = "Wait Input Event";
			Load += FormWaitInputEvent_Load;
			KeyDown += FormWaitInputEvent_KeyDown;
			ResumeLayout(false);
			PerformLayout();
		}

		#endregion

		private Label label1;
		private System.ComponentModel.BackgroundWorker backgroundWorker1;
	}
}