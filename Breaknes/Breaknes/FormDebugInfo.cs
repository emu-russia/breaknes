using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Collections;

using System.Runtime.InteropServices;

namespace Breaknes
{
    public partial class FormDebugInfo : Form
    {
        BreaksCore.DebugInfoType savedType;

        public FormDebugInfo(BreaksCore.DebugInfoType type)
        {
            savedType = type;

            InitializeComponent();
        }

        private void FormDebugInfo_Load(object sender, EventArgs e)
        {
            Text += " - " + savedType.ToString().Replace("DebugInfoType_", "");

            // Get debug entries of the selected type

            List<BreaksCore.DebugInfoEntry> entries = BreaksCore.GetDebugInfo(savedType);

            // Construct an object to show in PropertyGrid

            CustomClass myProperties = new CustomClass();

            foreach (BreaksCore.DebugInfoEntry entry in entries)
            {
                CustomProperty myProp = new CustomProperty();
                myProp.Name = entry.name;
                myProp.Category = entry.category;
                myProp.Value = entry.value;
                myProperties.Add (myProp);
            }

            propertyGrid1.SelectedObject = myProperties;
        }

        // Based on:
        // https://www.codeproject.com/Articles/9280/Add-Remove-Items-to-from-PropertyGrid-at-Runtime

        /// <summary>
        /// CustomClass (Which is binding to property grid)
        /// </summary>
        public class CustomClass : CollectionBase, ICustomTypeDescriptor
        {
            public AttributeCollection GetAttributes()
            {
                return TypeDescriptor.GetAttributes(this, true);
            }

            public string? GetClassName()
            {
                return TypeDescriptor.GetClassName(this, true);
            }

            public string? GetComponentName()
            {
                return TypeDescriptor.GetComponentName(this, true);
            }

            public TypeConverter GetConverter()
            {
                return TypeDescriptor.GetConverter(this, true);
            }

            public EventDescriptor? GetDefaultEvent()
            {
                return TypeDescriptor.GetDefaultEvent(this, true);
            }

            public PropertyDescriptor? GetDefaultProperty()
            {
                return TypeDescriptor.GetDefaultProperty(this, true);
            }

            public object? GetEditor(Type editorBaseType)
            {
                return TypeDescriptor.GetEditor(this, editorBaseType, true);
            }

            public EventDescriptorCollection GetEvents()
            {
                return TypeDescriptor.GetEvents(this, true);
            }

            public EventDescriptorCollection GetEvents(Attribute[]? attributes)
            {
                return TypeDescriptor.GetEvents(this, attributes, true);
            }

            public PropertyDescriptorCollection GetProperties()
            {
                return TypeDescriptor.GetProperties(this, true);
            }

            public PropertyDescriptorCollection GetProperties(Attribute[]? attributes)
            {
                PropertyDescriptor[] newProps = new PropertyDescriptor[this.Count];
                for (int i = 0; i < this.Count; i++)
                {
                    CustomProperty prop = (CustomProperty)base.List[i];
                    newProps[i] = new CustomPropertyDescriptor(ref prop, attributes);
                }

                return new PropertyDescriptorCollection(newProps);
            }

            public object? GetPropertyOwner(PropertyDescriptor? pd)
            {
                return this;
            }

            /// <summary>
            /// Add CustomProperty to Collectionbase List
            /// </summary>
            /// <param name="Value"></param>
            public void Add(CustomProperty Value)
            {
                base.List.Add(Value);
            }

            /// <summary>
            /// Remove item from List
            /// </summary>
            /// <param name="Name"></param>
            public void Remove(string Name)
            {
                foreach (CustomProperty prop in base.List)
                {
                    if (prop.Name == Name)
                    {
                        base.List.Remove(prop);
                        return;
                    }
                }
            }
        }

        public class CustomPropertyDescriptor : PropertyDescriptor
        {
            CustomProperty m_Property;
            public CustomPropertyDescriptor(ref CustomProperty myProperty,
                        Attribute[] attrs) : base(myProperty.Name, attrs)
            {
                m_Property = myProperty;
            }

            public override Type ComponentType => null;

            public override bool IsReadOnly => false;

            public override Type PropertyType => m_Property.Value.GetType();

            public override bool CanResetValue(object component)
            {
                return false;
            }

            public override object? GetValue(object? component)
            {
                return m_Property.Value;
            }

            public override void ResetValue(object component)
            {
                throw new NotImplementedException();
            }

            public override void SetValue(object? component, object? value)
            {
                m_Property.Value = value;
            }

            public override bool ShouldSerializeValue(object component)
            {
                return false;
            }
        }

        public class CustomProperty
        {
            public string Name { get; set; }
            public string Category { get; set; }
            public object Value { get; set; } = null;
        }

        public class CategoryAttribute : Attribute
        {
            CustomProperty m_Property;

            public CategoryAttribute (ref CustomProperty myProperty)
            {
                m_Property = myProperty;
            }

            public string GetName()
            {
                return m_Property.Category;
            }
        }


    }
}
