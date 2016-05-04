#include <iostream>
#include <cstdint>
#include <vector>
#include <memory>

#include <boost/any.hpp>

using Byte = std::uint8_t;

template<int Twidth>
class Fixed_format
{
public:
    static constexpr int width = Twidth;

    static constexpr int get_width()
    {
        return width;
    }
};

class Mutable_format
{
private:
    int m_width;
public:
    Mutable_format(const int width) :
        m_width(width)
    { }

    int get_width() const
    {
        return m_width;
    }
};

template<typename Input_format, typename Output_format>
std::vector<uint32_t> repack(
        const std::vector<uint32_t> &input,
        const Input_format &input_format,
        const Output_format &output_format)
{
    const int size = input.size();
    std::vector<uint32_t> output(size);
    const int shift = output_format.get_width() - input_format.get_width();
    if(shift > 0)
    {
        const int abs_shift = std::abs(shift);
        for(int i = 0; i < size; ++i)
        {
            output[i] = input[i] << abs_shift;
        }
    }
    else if(shift < 0)
    {
        const int abs_shift = std::abs(shift);
        for(int i = 0; i < size; ++i)
        {
            output[i] = input[i] >> abs_shift;
        }
    }
    else
    {
        for(int i = 0; i < size; ++i)
        {
            output[i] = input[i];
        }
    }
    return output;
}

class Format_visitor
{
public:
    virtual void visit(const Fixed_format<3> &format) = 0;
    virtual void visit(const Fixed_format<4> &format) = 0;
    virtual void visit(const Mutable_format &format) = 0;
};

template<typename TVisitor>
class Value_with_visitor
{
public:
    using Visitor = TVisitor;

private:
    class Holder_base
    {
    public:
        virtual void accept(Visitor &visitor) const = 0;
        virtual std::unique_ptr<Holder_base> clone() const = 0;
        virtual ~Holder_base() = 0;
    };

    template<typename Implementation>
    class Holder : public Holder_base
    {
    private:
        Implementation m_implementation;

    public:
        Holder(const Implementation &implementation) :
            m_implementation(implementation)
        { }

        void accept(Visitor &visitor) const override
        {
            visitor.visit(m_implementation);
        }

        std::unique_ptr<Holder_base> clone() const override
        {
            return std::unique_ptr<Holder_base>(new Holder(m_implementation));
        }

        ~Holder() = default;
    };

    std::unique_ptr<Holder_base> m_holder;

public:
    Value_with_visitor() = delete;

    template<typename Implementation>
    Value_with_visitor(const Implementation &implementation) :
        m_holder(new Holder<Implementation>(implementation))
    { }

    /* Need to declare the following two functions explicitly, so that version
     * using ordinary non-rvalue reference don't shadow these. */
    Value_with_visitor(Value_with_visitor &&other) = default;

    Value_with_visitor &operator=(Value_with_visitor &&other) = default;

    Value_with_visitor(const Value_with_visitor &other)
    {
        m_holder = other.m_holder->clone();
    }

    Value_with_visitor &operator=(const Value_with_visitor &other)
    {
        *this = Value_with_visitor(other);
        return *this;
    }

    void accept(Visitor &visitor) const
    {
        if(!m_holder.get())
            throw std::runtime_error(
                    "accessing empty format after move, undefined behaviour");
        m_holder->accept(visitor);
    }
};

template<typename Visitor>
Value_with_visitor<Visitor>::Holder_base::~Holder_base()
{ }

using Format = Value_with_visitor<Format_visitor>;

class Repack_proxy
{
public:
    using Input = std::vector<std::uint32_t>;
    using Output = Input;

private:
    template<typename TInput_format>
    class Output_selection : public Format::Visitor
    {
    public:
        using Input_format = TInput_format;
    private:
        const Input &m_input;
        const Input_format &m_input_format;
        Output m_output;

    public:
        Output_selection(
                const Input &input,
                const Input_format &input_format) :
            m_input(input),
            m_input_format(input_format)
        { }

        template<typename Output_format>
        void common_visit(const Output_format &output_format)
        {
            m_output = repack(m_input, m_input_format, output_format);
        }

        void visit(const Fixed_format<3> &input_format) override
        {
            common_visit(input_format);
        }

        void visit(const Fixed_format<4> &input_format) override
        {
            common_visit(input_format);
        }

        void visit(const Mutable_format &input_format) override
        {
            common_visit(input_format);
        }

        Output &&get_output()
        {
            return std::move(m_output);
        }
    };

    class Input_selection : public Format::Visitor
    {
    private:
        const Input &m_input;
        const Format &m_output_format;
        Output m_output;

    public:
        Input_selection(
                const Input &input,
                const Format &output_format) :
            m_input(input),
            m_output_format(output_format)
        { }

        template<typename Input_format>
        void common_visit(const Input_format &input_format)
        {
            Output_selection<Input_format> output_selection(
                        m_input,
                        input_format);
            m_output_format.accept(output_selection);
            m_output = output_selection.get_output();
        }

        void visit(const Fixed_format<3> &input_format) override
        {
            common_visit(input_format);
        }

        void visit(const Fixed_format<4> &input_format) override
        {
            common_visit(input_format);
        }

        void visit(const Mutable_format &input_format) override
        {
            common_visit(input_format);
        }

        Output &&get_output()
        {
            return std::move(m_output);
        }
    };

public:
    Output operator()(
            const Input &input,
            const Format &input_format,
            const Format &output_format)
    {
        Input_selection input_selection(input, output_format);
        input_format.accept(input_selection);
        return input_selection.get_output();
    }
};

std::vector<std::uint32_t> repack_adapter(
        const std::vector<uint32_t> &input,
        const Format &input_format,
        const Format &output_format)
{
    Repack_proxy proxy;
    return proxy(input, input_format, output_format);
}

void print(const std::vector<uint32_t> &input)
{
    for(const uint32_t i : input)
    {
        std::cout << i << ' ';
    }
    std::cout << '\n';
}

class Get_width : private Format::Visitor
{
private:
    int m_width = -1;

    void visit(const Fixed_format<3> &format) override
    {
        m_width = format.get_width();
    }

    void visit(const Fixed_format<4> &format) override
    {
        m_width = format.get_width();
    }

    void visit(const Mutable_format &format) override
    {
        m_width = format.get_width();
    }

public:
    int operator()(const Format &format)
    {
        m_width = -1;
        format.accept(*this);
        return m_width;
    }
};

int main()
{
    const std::vector<uint32_t> test_data{0x1, 0x2, 0x4, 0x8};
    std::cout << "test_data:\n";
    print(test_data);

    const std::vector<uint32_t> test_output =
            repack(test_data, Fixed_format<4>(), Mutable_format(6));
    std::cout << "test_output:\n";
    print(test_output);

    Format input_format = Fixed_format<4>();
    Format output_format = Mutable_format(6);

    const std::vector<uint32_t> test_output2 =
            repack_adapter(
                test_data,
                input_format,
                output_format);
    std::cout << "test_output2:\n";
    print(test_output2);

    Format format_copy = output_format;
    Get_width get_width;
    std::cout
            << "input width = " << get_width(input_format) << '\n'
            << "output width = " << get_width(output_format) << '\n'
            << "copy width = " << get_width(format_copy) << '\n';
    format_copy = input_format;
    std::cout << "altered copy width = " << get_width(format_copy) << '\n';
    input_format = std::move(format_copy);
    std::cout << "moved copy width = " << get_width(input_format) << '\n';
    try
    {
        std::cout << "undefined format width = " << get_width(format_copy) << '\n';
        std::cout << "Oh no, we are in an undefined state!\n";
    }
    catch(std::runtime_error &e)
    {
        std::cout << "Detected attempt of undefined behaviour.\n";
    }

    return 0;
}
