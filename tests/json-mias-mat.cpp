#include <json_tools.h>
#include "assert.h"

const char ingredientJsonError[] = R"json(
{
    "id": 666,
    "name": "Polser",
    "base_unit": "g",
    "description": "Fooofooofooo",
    "shop_group_id": 4,
    "energy_per_unit", 3.14,
    "vegetarian": true,
    "vegan": false,
    "grams_per_deciliter": 0.43,
    "grams_per_dimensionless": 0.0,
    "unit": "dl",
    "quantity": 2.0,
    "group_name": "topping",
    "recipe_id": 5,
    "recipe_name": "gryta",
    "use_ingredient_groups": true,
    "portions": 2,
    "portions_unit": "porsjon",
    "shop_group_name": "frukt und grunt",
    "allergens": [
      "fisk",
      "gluten"
    ]
}
)json";

const char shoppingListNameSkipJson[] = R"json(
{
    "ingredients": [
        {
            "id": 123,
            "recipe_name_id_list" : {
                "items" : []
            },
            "allergens" : [],
            "name" : "babyleafblader"
        }
    ],
    "userDefinedItems" : [
    ],
    "notes" : "",
    "fileVersion" : 2,
    "sortOrder" : 2,
    "name" : "Handleliste",
    "dateExplicit" : "9. november 2017",
    "timestamp" : "2017-11-09 21-52-05",
    "isAutomaticSave" : false
}
)json";

struct RecipeNameIdItem
{
    RecipeNameIdItem()
    {}

    RecipeNameIdItem(const std::string& recipe_name, int recipe_id)
        : recipe_name(recipe_name)
        , recipe_id(recipe_id)
    {}

    std::string recipe_name;
    int recipe_id;

    JT_STRUCT(
        JT_MEMBER(recipe_name),
        JT_MEMBER(recipe_id)
    );
};

struct RecipeNameIdList
{
    std::vector<RecipeNameIdItem> items;

    JT_STRUCT(
        JT_MEMBER(items));
};

struct ShoppingListItemCPP
{
    bool selected;

    JT_STRUCT(
        JT_MEMBER(selected));
};

struct IngredientCPP : public ShoppingListItemCPP
{
    int id;
    std::string name;
    std::string base_unit;
    std::string description;
    int shop_group_id;
    float energy_per_unit;
    bool vegetarian;
    bool vegan;
    float grams_per_deciliter;
    float grams_per_dimensionless;

    std::string unit;
    float quantity;
    std::string group_name;

    int recipe_id;
    std::string recipe_name;
    bool use_ingredient_groups;
    float portions;
    std::string portions_unit;

    std::string shop_group_name;

    std::vector<std::string> allergens;

    RecipeNameIdList recipe_name_id_list;

    JT_STRUCT_WITH_SUPER(
        JT_SUPER_CLASSES(JT_SUPER_CLASS(ShoppingListItemCPP)),
        JT_MEMBER(id),
        JT_MEMBER(name),
        JT_MEMBER(base_unit),
        JT_MEMBER(description),
        JT_MEMBER(shop_group_id),
        JT_MEMBER(energy_per_unit),
        JT_MEMBER(vegetarian),
        JT_MEMBER(vegan),
        JT_MEMBER(grams_per_deciliter),
        JT_MEMBER(grams_per_dimensionless),
        JT_MEMBER(unit),
        JT_MEMBER(quantity),
        JT_MEMBER(group_name),
        JT_MEMBER(recipe_id),
        JT_MEMBER(recipe_name),
        JT_MEMBER(use_ingredient_groups),
        JT_MEMBER(portions),
        JT_MEMBER(portions_unit),
        JT_MEMBER(shop_group_name),
        JT_MEMBER(allergens),
        JT_MEMBER(recipe_name_id_list)
    );
};

struct UserDefinedItemCPP : public ShoppingListItemCPP
{
    std::string text;

    JT_STRUCT_WITH_SUPER(
        JT_SUPER_CLASSES(JT_SUPER_CLASS(ShoppingListItemCPP)),
        JT_MEMBER(text)
    );
};

struct ShoppingListFileBase
{
    int fileVersion;
    int sortOrder;
    std::string name;
    std::string dateExplicit;
    std::string timestamp;
    bool isAutomaticSave;

    JT_STRUCT(
        JT_MEMBER(fileVersion),
        JT_MEMBER(sortOrder),
        JT_MEMBER(name),
        JT_MEMBER(dateExplicit),
        JT_MEMBER(timestamp),
        JT_MEMBER(isAutomaticSave)
    );
};

struct ShoppingListFileVersion02 : public ShoppingListFileBase
{
    std::vector<IngredientCPP> ingredients;
    std::vector<UserDefinedItemCPP> userDefinedItems;
    std::string notes;

    JT_STRUCT_WITH_SUPER(JT_SUPER_CLASSES(JT_SUPER_CLASS(ShoppingListFileBase)),
        JT_MEMBER(ingredients),
        JT_MEMBER(userDefinedItems),
        JT_MEMBER(notes)
    );
};

int main()
{
    IngredientCPP ingredient;
    JT::ParseContext pc(ingredientJsonError);
    pc.parseTo(ingredient);
    JT_ASSERT(pc.error == JT::Error::ExpectedDelimiter);

    ShoppingListFileBase fileBase;
    JT::ParseContext nameContext(shoppingListNameSkipJson);
    nameContext.parseTo(fileBase);
    JT_ASSERT(fileBase.name == "Handleliste");
}
