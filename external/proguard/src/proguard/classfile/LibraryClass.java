
package proguard.classfile;

import proguard.classfile.attribute.visitor.AttributeVisitor;
import proguard.classfile.constant.visitor.ConstantVisitor;
import proguard.classfile.util.*;
import proguard.classfile.visitor.*;

public class LibraryClass implements Clazz
{
    public int             u2accessFlags;
    public String          thisClassName;
    public String          superClassName;
    public String[]        interfaceNames;
    public LibraryField[]  fields;
    public LibraryMethod[] methods;

    /**
     * An extra field pointing to the superclass of this class.
     * This field is filled out by the {@link ClassSuperHierarchyInitializer}.
     */
    public Clazz   superClass;

    /**
     * An extra field pointing to the interfaces of this class.
     * This field is filled out by the {@link ClassSuperHierarchyInitializer}.
     */
    public Clazz[] interfaceClasses;

    /**
     * An extra field pointing to the subclasses of this class.
     * This field is filled out by the {@link ClassSubHierarchyInitializer}.
     */
    public Clazz[] subClasses;

    /**
     * An extra field in which visitors can store information.
     */
    public Object visitorInfo;


    /**
     * Creates an empty LibraryClass.
     */
    public LibraryClass() {}


    /**
     * Returns whether this library class is visible to the outside world.
     */
    boolean isVisible()
    {
        return (u2accessFlags & ClassConstants.INTERNAL_ACC_PUBLIC) != 0;
    }


    // Implementations for Clazz.

    public int getAccessFlags()
    {
        return u2accessFlags;
    }

    public String getName()
    {
        return thisClassName;
    }

    public String getSuperName()
    {
        // This may be java/lang/Object, in which case there is no super.
        return superClassName;
    }

    public int getInterfaceCount()
    {
        return interfaceClasses.length;
    }

    public String getInterfaceName(int index)
    {
        return interfaceNames[index];
    }

    public int getTag(int constantIndex)
    {
        throw new UnsupportedOperationException("Library class ["+thisClassName+"] doesn't store constant pool");
    }

    public String getString(int constantIndex)
    {
        throw new UnsupportedOperationException("Library class ["+thisClassName+"] doesn't store constant pool");
    }

    public String getStringString(int constantIndex)
    {
        throw new UnsupportedOperationException("Library class ["+thisClassName+"] doesn't store constant pool");
    }

    public String getClassName(int constantIndex)
    {
        throw new UnsupportedOperationException("Library class ["+thisClassName+"] doesn't store constant pool");
    }

    public String getName(int constantIndex)
    {
        throw new UnsupportedOperationException("Library class ["+thisClassName+"] doesn't store constant pool");
    }

    public String getType(int constantIndex)
    {
        throw new UnsupportedOperationException("Library class ["+thisClassName+"] doesn't store constant pool");
    }


    public void addSubClass(Clazz clazz)
    {
        if (subClasses == null)
        {
            subClasses = new Clazz[1];
        }
        else
        {
            // Copy the old elements into new larger array.
            Clazz[] temp     = new Clazz[subClasses.length+1];
            System.arraycopy(subClasses, 0, temp, 0, subClasses.length);
            subClasses = temp;
        }

        subClasses[subClasses.length-1] = clazz;
    }


    public Clazz getSuperClass()
    {
        return superClass;
    }


    public Clazz getInterface(int index)
    {
        return interfaceClasses[index];
    }


    public boolean extends_(Clazz clazz)
    {
        if (this.equals(clazz))
        {
            return true;
        }

        return superClass != null &&
               superClass.extends_(clazz);
    }


    public boolean extendsOrImplements(Clazz clazz)
    {
        if (this.equals(clazz))
        {
            return true;
        }

        if (superClass != null &&
            superClass.extendsOrImplements(clazz))
        {
            return true;
        }

        if (interfaceClasses != null)
        {
            for (int index = 0; index < interfaceClasses.length; index++)
            {
                Clazz interfaceClass = interfaceClasses[index];
                if (interfaceClass != null &&
                    interfaceClass.extendsOrImplements(clazz))
                {
                    return true;
                }
            }
        }

        return false;
    }


    public Field findField(String name, String descriptor)
    {
        for (int index = 0; index < fields.length; index++)
        {
            Field field = fields[index];
            if (field != null &&
                (name       == null || field.getName(this).equals(name)) &&
                (descriptor == null || field.getDescriptor(this).equals(descriptor)))
            {
                return field;
            }
        }

        return null;
    }


    public Method findMethod(String name, String descriptor)
    {
        for (int index = 0; index < methods.length; index++)
        {
            Method method = methods[index];
            if (method != null &&
                (name       == null || method.getName(this).equals(name)) &&
                (descriptor == null || method.getDescriptor(this).equals(descriptor)))
            {
                return method;
            }
        }

        return null;
    }


    public void accept(ClassVisitor classVisitor)
    {
        classVisitor.visitLibraryClass(this);
    }


    public void hierarchyAccept(boolean      visitThisClass,
                                boolean      visitSuperClass,
                                boolean      visitInterfaces,
                                boolean      visitSubclasses,
                                ClassVisitor classVisitor)
    {
        // First visit the current classfile.
        if (visitThisClass)
        {
            accept(classVisitor);
        }

        // Then visit its superclass, recursively.
        if (visitSuperClass)
        {
            if (superClass != null)
            {
                superClass.hierarchyAccept(true,
                                           true,
                                           visitInterfaces,
                                           false,
                                           classVisitor);
            }
        }

        // Then visit its interfaces, recursively.
        if (visitInterfaces)
        {
            // Visit the interfaces of the superclasses, if we haven't done so yet.
            if (!visitSuperClass)
            {
                if (superClass != null)
                {
                    superClass.hierarchyAccept(false,
                                               false,
                                               true,
                                               false,
                                               classVisitor);
                }
            }

            // Visit the interfaces.
            if (interfaceClasses != null)
            {
                for (int index = 0; index < interfaceClasses.length; index++)
                {
                    Clazz interfaceClass = interfaceClasses[index];
                    if (interfaceClass != null)
                    {
                        interfaceClass.hierarchyAccept(true,
                                                       false,
                                                       true,
                                                       false,
                                                       classVisitor);
                    }
                }
            }
        }

        // Then visit its subclasses, recursively.
        if (visitSubclasses)
        {
            if (subClasses != null)
            {
                for (int index = 0; index < subClasses.length; index++)
                {
                    subClasses[index].hierarchyAccept(true,
                                                      false,
                                                      false,
                                                      true,
                                                      classVisitor);
                }
            }
        }
    }


    /**
     * Lets the given class visitor visit the superclass, if it is known.
     * @param classVisitor the <code>ClassVisitor</code> that will visit the
     *                     superclass.
     */
    public void superClassAccept(ClassVisitor classVisitor)
    {
        if (superClass != null)
        {
            superClass.accept(classVisitor);
        }
    }


    /**
     * Lets the given class visitor visit all known direct interfaces.
     * @param classVisitor the <code>ClassVisitor</code> that will visit the
     *                     interfaces.
     */
    public void interfacesAccept(ClassVisitor classVisitor)
    {
        if (interfaceClasses != null)
        {
            for (int index = 0; index < interfaceClasses.length; index++)
            {
                Clazz interfaceClass = interfaceClasses[index];
                if (interfaceClass != null)
                {
                    interfaceClass.accept(classVisitor);
                }
            }
        }
    }


    public void subclassesAccept(ClassVisitor classVisitor)
    {
        if (subClasses != null)
        {
            for (int index = 0; index < subClasses.length; index++)
            {
                subClasses[index].accept(classVisitor);
            }
        }
    }


    public void constantPoolEntriesAccept(ConstantVisitor constantVisitor)
    {
        // This class doesn't keep references to its constant pool entries.
    }


    public void constantPoolEntryAccept(int index, ConstantVisitor constantVisitor)
    {
        // This class doesn't keep references to its constant pool entries.
    }


    public void thisClassConstantAccept(ConstantVisitor constantVisitor)
    {
        // This class doesn't keep references to its constant pool entries.
    }


    public void superClassConstantAccept(ConstantVisitor constantVisitor)
    {
        // This class doesn't keep references to its constant pool entries.
    }


    public void interfaceConstantsAccept(ConstantVisitor constantVisitor)
    {
        // This class doesn't keep references to its constant pool entries.
    }


    public void fieldsAccept(MemberVisitor memberVisitor)
    {
        for (int index = 0; index < fields.length; index++)
        {
            Field field = fields[index];
            if (field != null)
            {
                field.accept(this, memberVisitor);
            }
        }
    }


    public void fieldAccept(String name, String descriptor, MemberVisitor memberVisitor)
    {
        Field field = findField(name, descriptor);
        if (field != null)
        {
            field.accept(this, memberVisitor);
        }
    }


    public void methodsAccept(MemberVisitor memberVisitor)
    {
        for (int index = 0; index < methods.length; index++)
        {
            Method method = methods[index];
            if (method != null)
            {
                method.accept(this, memberVisitor);
            }
        }
    }


    public void methodAccept(String name, String descriptor, MemberVisitor memberVisitor)
    {
        Method method = findMethod(name, descriptor);
        if (method != null)
        {
            method.accept(this, memberVisitor);
        }
    }


    public boolean mayHaveImplementations(Method method)
    {
        return
           (u2accessFlags & ClassConstants.INTERNAL_ACC_FINAL) == 0 &&
           (method == null ||
            ((method.getAccessFlags() & (ClassConstants.INTERNAL_ACC_PRIVATE |
                                         ClassConstants.INTERNAL_ACC_STATIC  |
                                         ClassConstants.INTERNAL_ACC_FINAL)) == 0 &&
                                                                                  !method.getName(this).equals(ClassConstants.INTERNAL_METHOD_NAME_INIT)));
    }


    public void attributesAccept(AttributeVisitor attributeVisitor)
    {
        throw new UnsupportedOperationException("Library class ["+thisClassName+"] doesn't store attributes");
    }


    // Implementations for VisitorAccepter.

    public Object getVisitorInfo()
    {
        return visitorInfo;
    }

    public void setVisitorInfo(Object visitorInfo)
    {
        this.visitorInfo = visitorInfo;
    }


    // Implementations for Object.

    public String toString()
    {
        return "LibraryClass("+getName()+")";
    }
}
