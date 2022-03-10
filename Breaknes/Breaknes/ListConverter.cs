using System.Text;
using System.Globalization;
using System.ComponentModel;
using System.Collections.Generic;
using System.Collections;

namespace Breaknes
{

    // https://stackoverflow.com/questions/32582504/propertygrid-expandable-collection

    public class ListConverter : CollectionConverter
    {
#pragma warning disable CS8765 // Nullability of type of parameter doesn't match overridden member (possibly because of nullability attributes).
        public override bool GetPropertiesSupported(ITypeDescriptorContext context)
#pragma warning restore CS8765 // Nullability of type of parameter doesn't match overridden member (possibly because of nullability attributes).
        {
            return true;
        }

#pragma warning disable CS8765 // Nullability of type of parameter doesn't match overridden member (possibly because of nullability attributes).
        public override PropertyDescriptorCollection GetProperties(ITypeDescriptorContext context, object value, Attribute[] attributes)
#pragma warning restore CS8765 // Nullability of type of parameter doesn't match overridden member (possibly because of nullability attributes).
        {
            IList? list = value as IList;
            if (list == null || list.Count == 0)
                return base.GetProperties(context, value, attributes);

            var items = new PropertyDescriptorCollection(null);
            for (int i = 0; i < list.Count; i++)
            {
#pragma warning disable CS8600 // Converting null literal or possible null value to non-nullable type.
                object item = list[i];
#pragma warning restore CS8600 // Converting null literal or possible null value to non-nullable type.
                items.Add(new ExpandableCollectionPropertyDescriptor(list, i));
            }
            return items;
        }

        public class ExpandableCollectionPropertyDescriptor : PropertyDescriptor
        {
            private IList collection;
            private readonly int _index;

            public ExpandableCollectionPropertyDescriptor(IList coll, int idx)
                : base(GetDisplayName(coll, idx), null)
            {
                collection = coll;
                _index = idx;
            }

            private static string GetDisplayName(IList list, int index)
            {
                if (list != null)
                {
#pragma warning disable CS8602 // Dereference of a possibly null reference.
                    return "[" + index + "]  " + CSharpName(list[index].GetType());
#pragma warning restore CS8602 // Dereference of a possibly null reference.
                }
                else
                {
                    return "<null>";
                }
            }

            private static string CSharpName(Type type)
            {
                var sb = new StringBuilder();
                var name = type.Name;
                if (!type.IsGenericType)
                    return name;
                sb.Append(name.Substring(0, name.IndexOf('`')));
                sb.Append("<");
                sb.Append(string.Join(", ", type.GetGenericArguments().Select(CSharpName)));
                sb.Append(">");
                return sb.ToString();
            }

            public override bool CanResetValue(object component)
            {
                return true;
            }

            public override Type ComponentType
            {
                get { return this.collection.GetType(); }
            }

#pragma warning disable CS8765 // Nullability of type of parameter doesn't match overridden member (possibly because of nullability attributes).
            public override object GetValue(object component)
#pragma warning restore CS8765 // Nullability of type of parameter doesn't match overridden member (possibly because of nullability attributes).
            {
#pragma warning disable CS8603 // Possible null reference return.
                return collection[_index];
#pragma warning restore CS8603 // Possible null reference return.
            }

            public override bool IsReadOnly
            {
                get { return false; }
            }

            public override string Name
            {
                get { return _index.ToString(CultureInfo.InvariantCulture); }
            }

            public override Type PropertyType
            {
#pragma warning disable CS8602 // Dereference of a possibly null reference.
                get { return collection[_index].GetType(); }
#pragma warning restore CS8602 // Dereference of a possibly null reference.
            }

            public override void ResetValue(object component)
            {
            }

            public override bool ShouldSerializeValue(object component)
            {
                return true;
            }

#pragma warning disable CS8765 // Nullability of type of parameter doesn't match overridden member (possibly because of nullability attributes).
            public override void SetValue(object component, object value)
#pragma warning restore CS8765 // Nullability of type of parameter doesn't match overridden member (possibly because of nullability attributes).
            {
                collection[_index] = value;
            }
        }
    }

}
