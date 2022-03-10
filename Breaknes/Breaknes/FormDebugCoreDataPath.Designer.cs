namespace Breaknes
{
    partial class FormDebugCoreDataPath
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
            this.dataPathView1 = new Breaknes.DataPathView();
            this.SuspendLayout();
            // 
            // dataPathView1
            // 
            this.dataPathView1.BackgroundImage = global::Breaknes.Properties.Resources.datapath;
            this.dataPathView1.Location = new System.Drawing.Point(12, 12);
            this.dataPathView1.Name = "dataPathView1";
            this.dataPathView1.Size = new System.Drawing.Size(857, 342);
            this.dataPathView1.TabIndex = 0;
            // 
            // FormDebugCoreDataPath
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(867, 348);
            this.Controls.Add(this.dataPathView1);
            this.Name = "FormDebugCoreDataPath";
            this.Text = "FormDebugCoreDataPath";
            this.ResumeLayout(false);

        }

        #endregion

        private DataPathView dataPathView1;
    }
}